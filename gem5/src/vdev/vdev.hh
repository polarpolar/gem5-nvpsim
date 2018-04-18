//
// 	Head file for Virtual Device
// 
//	Version 2.1.	12/3/2017, tongda
//		Add new states considering initializations
//	Perivious versions: ruoyang
//		Add access time and log need functions
//	Created by lf-z on 4/24/17
//

#ifndef GEM5_VDEV_HH
#define GEM5_VDEV_HH

#include <fstream>

#include <string>
#include <vector>
#include "mem/mem_object.hh"
#include "sim/eventq.hh"
#include "cpu/base.hh"
#include "params/VirtualDevice.hh"

class VirtualDevice : public MemObject
{

protected:
	/** Id of the virtual device */
	uint32_t id;
	char dev_name[100];

private:
	/** Whether the state needs recover **/
	bool need_recover;

	/** Definitions of TickEvent in Virtual Device **/
	/** The behavior is constructed in tick-level */
	struct TickEvent : public Event
	{
		VirtualDevice *vdev;

		TickEvent(VirtualDevice *vdev_this);
		void process();
		const char *description() const;
	};

	TickEvent tickEvent;

	/** Record the execution state and energy consumption. **/
	void tick();

	/** Definitions of Device Port in Virtual Device **/
	/** Device Port is a slave Energy Port of vdev to connect to the Energy framework. */
	class DevicePort : public SlavePort
	{
	private:
		VirtualDevice* vdev;

	public:
		DevicePort(const std::string& _name, VirtualDevice* _vdev);

	protected:
		Tick recvAtomic(PacketPtr pkt);
		void recvFunctional(PacketPtr pkt);
		bool recvTimingReq(PacketPtr pkt);
		void recvRespRetry();
		AddrRangeList getAddrRanges() const;
	};

	DevicePort port;

public:

	typedef VirtualDeviceParams Params;
	const Params *params() const
	{
		return reinterpret_cast<const Params *>(_params);
	}
	VirtualDevice(const Params *p);
	virtual ~VirtualDevice();
	virtual void init();

	/* Flags defined be the first byte in the memory. 
	 * |--high-----------------|---------------------low--|
	 * |------R & W---------|-------Read only---------|
	 * |Init|Activate|---------|Chaos|Ready|Busy|Idle|
	 
	 R/W command: (configurable)
	 Init: 		Initialize the virtual device
	 Activate: 	activate a peripheral operation

	 Virtual device states: (read only)
	 chaos 	: device uninitialized
	 ready 	: device is initialized
	 busy 	: device is carrying on a peripheral operation/initialization procedure
	 idle 	: device is ready to carry on a new operation

	*/
	static const uint8_t VDEV_INIT		= 0x80;
	static const uint8_t VDEV_ACTIVATE	= 0x40;
	static const uint8_t VDEV_CHAOS		= 0x08;
	static const uint8_t VDEV_READY		= 0x04;
	static const uint8_t VDEV_BUSY		= 0x02;
	static const uint8_t VDEV_IDLE		= 0x01;

	/** Method to trigger an interrupt after task finishes. */
	void triggerInterrupt();
	/** Simple method to access data. */
	Tick access(PacketPtr pkt);
	/** Handle energy state changes. */
	virtual int handleMsg(const EnergyMsg &msg);
	/** Method for python scripts to get port. */
	BaseSlavePort& getSlavePort(const std::string& if_name, PortID idx = InvalidPortID);
	/** Method to get addr range. */
	AddrRange getAddrRange() const;

	/** Methods to handle packet. */
	Tick recvAtomic(PacketPtr pkt);
	void recvFunctional(PacketPtr pkt);
	bool recvTimingReq(PacketPtr pkt);
	void recvRespRetry();

	/** Vdev Energy State related parameters **/
	/* Three states are defined as energy modes, that are: 
	 	power-off 	: totally fail, execution state = raw;
	 	sleep  		: low power mode, only keep the status, cannot execute operations
	 	active 	 	: full power mode, can carry on all the operations
	*/
	enum VdevEngyState {
		STATE_POWER_OFF 	= 0,
		STATE_SLEEP 		= 1,
		STATE_NORMAL 		= 2,
		STATE_ACTIVE 		= 3
	};

	bool need_log;
	int access_time = 0;

	/*inline Tick clockPeriod() const
	{
		return ClockedObject::clockPeriod() * clkmult;
	}

	inline Cycles ticksToCycles(Tick t) const
	{
		return ClockedObject::ticksToCycles(t / clkmult);
	}*/


protected:
	/** CPU of the device **/
	BaseCPU *cpu;
	/** Address range of the virtual device*/
	AddrRange range;
	/** Trace of the vdev, used to determine whether a task has succeeded */
	std::vector<std::string> trace;
	/** The delay for the CPU to set the vdev */
	Tick delay_set;
	/** The delay for the vdev to finish its task */
	Tick delay_self;
	/** Time that the cpu spend to recover the device after a power failure */
	Tick delay_recover;
	/** The delay for the CPU to handle the interrupt caused by the vdev */
	Tick delay_cpu_interrupt;
	/** Whether the tasks of the vdev is interruptable */
	bool is_interruptable;
	/** When power off, time is remained for the task */
	Tick delay_remained;
	/** Energy consumption in different modes : (*1000?)**/
	double energy_consumed_per_cycle_vdev[4] = {0, 0.01, 0.2, 2};
	/** Energy modes of vdev : [OFF, SLEEP, NORMAL, ACTIVE]; **/
	VdevEngyState vdev_energy_state;

	/** The normal/init interrupt event scheduled by vdev **/
	EventWrapper<VirtualDevice, &VirtualDevice::triggerInterrupt> event_interrupt;
	EventWrapper<VirtualDevice, &VirtualDevice::triggerInterrupt> event_init;
	/** Tell whether the task is successful */
	virtual bool finishSuccess();
	/** Implement of memories and registers for the vdev. */
	uint8_t *pmem;

};

#endif //GEM5_VDEV_HH
