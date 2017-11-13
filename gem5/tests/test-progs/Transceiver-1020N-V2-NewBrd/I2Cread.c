//////  I2C read		////////
	
	//PRER set to 400kHz
	P1 = 0xD0;		//11010000,PRERlo
	P0 = 0x07;		
	P1 = 0x54;
	P0 = 0x00;
	P1 = 0xD1;		//11010001,PRERhi

  //TXR: transfer slave adr
	P1 = 0x54;
	P0 = 0x20;
	P1 = 0xD3;		//11010011,TXR
	
	//CR: start, write
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0xD4;		//11010100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	P2 = 0x00;		
	while(TIP);
	P2 = 0xDc;		// flag

	
	
	//TXR: slave mem adr
	P1 = 0x54;
	P0 = 0x01;
	P1 = 0xD3;		//11010011,TXR
	
	//CR: write
	P1 = 0x54;
	P0 = 0x10;
	P1 = 0xD4;		//11010100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	P2 = 0x00;
	while(TIP);
	P2 = 0xfc;		// flag
	
	//TXR: slave adr
	P1 = 0x54;
	P0 = 0x21;
	P1 = 0xD3;			
	//CR: restart, read
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0xD4;			
	//SR: wait tranfer over
	P1 = 0x54;
	P2 = 0x00;
	while(TIP);
	P2 = 0xDc;		// flag
	
	//CR:read,ack
	P1 = 0x54;
	P0 = 0x20;
	P1 = 0xD4;		
	
	//SR: wait tranfer over
	P1 = 0x54;
	P2 = 0x00;
	while(TIP);
	P2 = 0xfc;		// flag
	
	//RXR
	P1 = 0x53;
	P2 = 0x00;		// insert nop
	if(P0!=0x11)	P2 = 0xee;		// flag

//CR:read,ack
	P1 = 0x54;
	P0 = 0x20;
	P1 = 0x74;		
	
	//SR: wait tranfer over
	P1 = 0x54;
	P2 = 0x00;
	while(TIP);
	P2 = 0x7c;		// flag
	
	//RXR
	P1 = 0x53;
	P2 = 0x00;
	if(P0!=0x22)	P2 = 0xee;		// flag
	
	//CR:read,nack,stop
	P1 = 0x54;
	P0 = 0x68;
	P1 = 0x74;		
	
	//SR: wait tranfer over
	P1 = 0x54;
	P2 = 0x00;
	while(TIP);
	P2 = 0xfc;		// flag
	
	//RXR
	P1 = 0x53;
	P2 = 0x00;
	if(P0!=0x33)	P2 = 0xee;			// flag
	P2 = 0xf8;		// flag
