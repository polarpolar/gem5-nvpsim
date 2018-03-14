
int
VirtualDevice::handleMsg(const EnergyMsg &msg)
{
	switch(msg.type) {
		case (int) SM_TwoThres::MsgType::RETENTION_BEG:
			/** VDEV retention **
			  The virtual device energy mode turns to sleep (low cost), but all the uncompleted tasks are failed. Recover delay contains only 'delay_self' is it is un-interruptable.
			**/
			DPRINTF(EnergyMgmt, "%s receives Enter-Retention Msg at %lu, msg.type=%d\n", dev_name, curTick(), msg.type);
			vdev_energy_state = STATE_SLEEP;
			if (*pmem & VDEV_BUSY) {
				/* This should be handled if the device is on a task */
				assert(event_interrupt.scheduled());
				DPRINTF(VirtualDevice, "Working to Retention, %s state: Retention\n", dev_name);

				/* Calculate the remaining delay*/
				if (!is_interruptable) {
					// the interrupted operation is initialization
					if (*pmem & VDEV_RAW)
						delay_remained = delay_set;
					// the interrupted operation is execution
					else
						delay_remained = delay_self;
				} else {
					delay_remained = event_interrupt.when() - curTick();
				}
				deschedule(event_interrupt);
			}
			break;
		case (int) SM_TwoThres::MsgType::POWEROFF:
			/** VDEV shutdown **
			  The virtual device completed failed. It becomes un-initialized and needs re-initialization. If it is un-interruptable and fails during busy state, it also needs to restart and rerun the task with 'delay_self'. The device requires a recover time ('delay_recover') to support the reinitialization and restart procedure. 
			**/
			DPRINTF(EnergyMgmt, "%s receives Enter-Power-Off Msg at %lu, msg.type=%d\n", dev_name, curTick(), msg.type);
			/* Reset the states */
			*pmem |= VDEV_RAW;
			*pmem &= ~VDEV_READY;
			vdev_energy_state = STATE_POWEROFF;
			/* Todo: if entered from retention */
			if (vdev_energy_state == STATE_SLEEP) {
				assert(!event_interrupt.scheduled());
				DPRINTF(VirtualDevice, "Retention to OFF, %s state: Power-off\n", dev_name);
				/* delay_remained updates with punishment */
				Tick delay_punish = 0;
				delay_remained += delay_recover + delay_set + delay_punish;
			} 
			/* if entered from active modes */
			else {
				delay_remained = delay_recover + delay_set;
				if (*pmem & VDEV_BUSY) {
					assert(event_interrupt.scheduled());
					DPRINTF(VirtualDevice, "Working to OFF, %s state: Power-off\n", dev_name);
					if (!is_interruptable) {
						delay_remained += delay_self;
					} else {
						/*Todo: what if the peripheral is interruptable*/
						delay_remained += event_interrupt.when() - curTick();
					}
					deschedule(event_interrupt);
				}
			}	
			break;
		case (int) SM_TwoThres::MsgType::RETENTION_END:
			/** VDEV recover from retention **/
			DPRINTF(EnergyMgmt, "%s receives Retention-to-Active Msg at %lu, msg.type=%d\n", dev_name, curTick(), msg.type);
			vdev_energy_state = STATE_ACTIVE;
			if (*pmem & VDEV_BUSY) 
			{
				assert(!event_interrupt.scheduled());
				schedule(event_interrupt, curTick() + delay_remained);
				/* Energy consumption. */
				DPRINTF(VirtualDevice, "Recover from retention, Need %i Cycles, Energy: %lf, %s state: Active\n", 
					ticksToCycles(delay_remained), 
					energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained),
					dev_name
				);
				//consumeEnergy(dev_name, energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained));
				cpu->virtualDeviceStart(id);
			}
			break;
		case (int) SM_TwoThres::MsgType::POWERON:
			/** VDEV power recover **/
			DPRINTF(EnergyMgmt, "The Msg is Poweroff-to-Active.\n");
			vdev_energy_state = STATE_ACTIVE;
			if (*pmem & VDEV_BUSY) {
				assert(!event_interrupt.scheduled());
				schedule(event_interrupt, curTick() + delay_remained);
				/** Energy consumption **/
				DPRINTF(VirtualDevice, "Start working, Need %i Cycles, Energy: %lf, %s state: Active\n",  
					ticksToCycles(delay_remained), 
					energy_consumed_per_cycle_vdev[STATE_ACTIVE] * ticksToCycles(delay_remained),
					dev_name
				);
				cpu->virtualDeviceStart(id);
			}
			break;
		default:
			DPRINTF(EnergyMgmt, "Unrecognized EngyMsg.\n");
			return 0;
	}
	return 1;
}