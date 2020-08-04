#include <common.h>
#include <stdio.h>

#include "sensor_power.h"
#include "isp_api.h"
#include "i2c_api.h"
#include "isp_api_s.h"

#define BYPASSCDSP_NEW_RAW10	(0)
#define BYPASSCDSP_RAW8			(0)
#define CDSP_SCALER_HD			(0) //scale down from FHD  to HD size		
#define CDSP_SCALER_VGA			(0) //scale down from FHD  to VGA size
#define CDSP_SCALER_QQVGA		(0) //scale down from FHD  to QVGA size
#define COLOR_BAR_MOVING		(0)
#define INTERRUPT_VS_FALLING	(0)

//front table and struct
#define MAX_FRONT_REG_LEN		(100)

unsigned char FRONT_INIT_FILE[] = {
	#include "FrontInit_s.txt"
};

typedef struct 
{
	u8 type; //reversed
	u16 adr;
	u8 dat;	
} FRONT_DATA_T;

typedef struct
{
	u16 count; //0x00, 0x19, => 0x0019=25
	FRONT_DATA_T FRONT_DATA[MAX_FRONT_REG_LEN];
	//FRONT_DATA_T FRONT_DATA[(u16)(ARRAY_SIZE(FRONT_INIT_FILE)/sizeof(FRONT_DATA_T))];
} FRONT_INIT_FILE_T;

//cdsp table and struct
#define MAX_CDSP_REG_LEN		(300)

unsigned char CDSP_INIT_FILE[] = {
	#include "CdspInit_s.txt"
};

typedef struct
{
	u8 type; //reversed
	u16 adr;
	u8 dat;	
} CDSP_DATA_T;

typedef struct
{
	u16 count; //0x01, 0x02, => 0x0102=258
	CDSP_DATA_T CDSP_DATA[MAX_CDSP_REG_LEN];
	//CDSP_DATA_T CDSP_DATA[(u16)(ARRAY_SIZE(CDSP_INIT_FILE)/sizeof(CDSP_DATA_T))];
} CDSP_INIT_FILE_T;

//sensor struct
#define SC2310_DEVICE_ADDR		(0x60)
#define MAX_SENSOR_REG_LEN		(400)

unsigned char SENSOR_INIT_FILE[] = {
	#include "SensorInit_s.txt"
};

typedef struct
{
	u8 type; //0xFE:do delay in ms , 0x00:sensor register 
	//u16 adr;
	//u16 dat;
	
	//u8 adr;
	//u16 dat;
	
	u16 adr;
	u8 dat;
	
	//u8 adr;
	//u8 dat;
	//PS. By different sensor type, adr or dat could be one or two bytes.
	//ex.
	/*
	0xFE, 0x00, 0xC8, 0x00, 0x00, //DELAY= 200ms
	0x00, 0x30, 0x1A, 0x10, 0xD8, //sensor addr=0x301a, sensor data=0x10d8
	*/	
} SENSOR_DATA_T;

typedef struct
{
	u16 count; //0x01, 0x60, => 0x0160=352
	SENSOR_DATA_T SENSOR_DATA[MAX_SENSOR_REG_LEN];
	//SENSOR_DATA_T SENSOR_DATA[(u16)(ARRAY_SIZE(SENSOR_INIT_FILE)/sizeof(SENSOR_DATA_T))];
} SENSOR_INIT_FILE_T;

unsigned char SF_FIXED_PATTERN_NOISE_S[]={
	#include "FixedPatternNoise_s.txt"
};
unsigned char SF_LENS_COMP_B_S[]={
	#include "lenscompb_s.txt"
};
unsigned char SF_LENS_COMP_G_S[]={
	#include "lenscompg_s.txt"
};
unsigned char SF_LENS_COMP_R_S[]={
	#include "lenscompr_s.txt"
};
unsigned char SF_POST_GAMMA_B_S[]={
	#include "PostGammaB_s.txt"
};
unsigned char SF_POST_GAMMA_G_S[]={
	#include "PostGammaG_s.txt"
};
unsigned char SF_POST_GAMMA_R_S[]={
	#include "PostGammaR_s.txt"
};

void sensorDump(void)
{
	SENSOR_INIT_FILE_T read_file;
	int i, total;
	u16 dat = 0xff;
	u8 data[4];

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	// conver table data to SENSOR_INIT_FILE_T struct
	total = ARRAY_SIZE(SENSOR_INIT_FILE);
	data[0] = SENSOR_INIT_FILE[0];
	data[1] = SENSOR_INIT_FILE[1];
	data[2] = SENSOR_INIT_FILE[2];
	data[3] = SENSOR_INIT_FILE[3];
	read_file.count = (data[0]<<8)|data[1];

	ISPAPB_LOGI("%s, total=%d\n", __FUNCTION__, total);
	ISPAPB_LOGI("%s, count=%d, data[0]=0x%02x, data[1]=0x%02x, data[2]=0x%02x, data[3]=0x%02x\n", __FUNCTION__, read_file.count, data[0], data[1], data[2], data[3]);
	for (i = 2; i < total; i=i+4)
	{
		data[0] = SENSOR_INIT_FILE[i];
		data[1] = SENSOR_INIT_FILE[i+1];
		data[2] = SENSOR_INIT_FILE[i+2];
		data[3] = SENSOR_INIT_FILE[i+3];
		read_file.SENSOR_DATA[(i-2)/4].type = data[0];
		read_file.SENSOR_DATA[(i-2)/4].adr	= (data[1]<<8)|data[2];
		read_file.SENSOR_DATA[(i-2)/4].dat	= data[3];
		//ISPAPB_LOGI("%s, i=%d, data[0]=0x%02x, data[1]=0x%02x, data[2]=0x%02x, data[3]=0x%02x\n", __FUNCTION__, i, data[0], data[1], data[2], data[3]);
	}

	I2CReset();
	I2CInit(SC2310_DEVICE_ADDR, 0);

	ISPAPB_LOGI("%s, count=%d\n", __FUNCTION__, read_file.count);
	for (i = 0; i < read_file.count; i++)
	{
		if (read_file.SENSOR_DATA[i].type == 0xFE)
		{
			//udelay(read_file.SENSOR_DATA[i].adr*1000);
		}
		else if (read_file.SENSOR_DATA[i].type == 0x00)
		{
			dat = 0xff;
			getSensor16_I2C1((unsigned long)read_file.SENSOR_DATA[i].adr, &dat, 1);
			ISPAPB_LOGI("%s, type=0x%02x, adr=0x%04x, dat=0x%04x\n", __FUNCTION__, read_file.SENSOR_DATA[i].type, read_file.SENSOR_DATA[i].adr, dat);
		}
	}

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

/*
	@ispSleep_s this function depends on O.S.
*/
void ispSleep_s(void)
{
	int i;

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	for (i = 0;i < 10; i++) {
		MOON0_REG->clken[2];
	}

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

/*
	@ispReset_s
*/
void ispReset_s(void)
{
	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	ISPAPB0_REG8(0x2000) = 0x13; //reset all module include ispck
    ISPAPB0_REG8(0x2003) = 0x1c; //enable phase clocks
    ISPAPB0_REG8(0x2005) = 0x07; //enbale p1xck
    ISPAPB0_REG8(0x2008) = 0x05; //switch b1xck/bpclk_nx to normal clocks
    ISPAPB0_REG8(0x2000) = 0x03; //release ispck reset
	ispSleep_s();//#(`TIMEBASE*20;
	//
	ISPAPB0_REG8(0x2000) = 0x00; //release all module reset
	//
	ISPAPB0_REG8(0x276c) = 0x01; //reset front
	ISPAPB0_REG8(0x276c) = 0x00; //
	//	
	ISPAPB0_REG8(0x2000) = 0x03;
	ISPAPB0_REG8(0x2000) = 0x00;
	//
	ISPAPB0_REG8(0x2010) = 0x00;//cclk: 48MHz

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

/*
	@FrontInit_s
	ex. FrontInit_s(1920, 1080);
*/
void FrontInit_s(int width, int height)
{
	//FRONT_INIT_FILE_T *read_file = (FRONT_INIT_FILE_T *)FRONT_INIT_FILE;
	FRONT_INIT_FILE_T read_file;
	int i, total;
	u8 data[4];

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

 	// conver table data to FRONT_INIT_FILE_T struct
	total = ARRAY_SIZE(FRONT_INIT_FILE);
	data[0] = FRONT_INIT_FILE[0];
	data[1] = FRONT_INIT_FILE[1];
	data[2] = FRONT_INIT_FILE[2];
	data[3] = FRONT_INIT_FILE[3];
	read_file.count = (data[0]<<8)|data[1];

	ISPAPB_LOGI("%s, total=%d\n", __FUNCTION__, total);
	ISPAPB_LOGI("%s, count=%d, data[0]=0x%02x, data[1]=0x%02x, data[2]=0x%02x, data[3]=0x%02x\n", __FUNCTION__, read_file.count, data[0], data[1], data[2], data[3]);
	for (i = 2; i < total; i=i+4)
	{
		data[0] = FRONT_INIT_FILE[i];
		data[1] = FRONT_INIT_FILE[i+1];
		data[2] = FRONT_INIT_FILE[i+2];
		data[3] = FRONT_INIT_FILE[i+3];
		read_file.FRONT_DATA[(i-2)/4].type = data[0];
		read_file.FRONT_DATA[(i-2)/4].adr  = (data[1]<<8)|data[2];
		read_file.FRONT_DATA[(i-2)/4].dat  = data[3];
		//ISPAPB_LOGI("%s, i=%d, data[0]=0x%02x, data[1]=0x%02x, data[2]=0x%02x, data[3]=0x%02x\n", __FUNCTION__, i, data[0], data[1], data[2], data[3]);
	}

	//clock setting
	ISPAPB0_REG8(0x2008) = 0x07;

	ISPAPB_LOGI("%s, count=%d\n", __FUNCTION__, read_file.count);
	for (i = 0; i < read_file.count; i++)
	{
		//ISPAPB_LOGI("%s, type=0x%02x, adr=0x%04x, dat=0x%04x\n", __FUNCTION__, read_file.FRONT_DATA[i].type, read_file.FRONT_DATA[i].adr, read_file.FRONT_DATA[i].dat);
		if (read_file.FRONT_DATA[i].type == 0x00)
		{
			ISPAPB0_REG8((unsigned long)read_file.FRONT_DATA[i].adr) = read_file.FRONT_DATA[i].dat;
		}
	}
	//
	ISPAPB0_REG8(0x2720) = 0x00;	/* hoffset */
	ISPAPB0_REG8(0x2721) = 0x00;
	ISPAPB0_REG8(0x2722) = 0x00;	/* voffset */
	ISPAPB0_REG8(0x2723) = 0x00;
	ISPAPB0_REG8(0x2724) = 0x84;	/* hsize */
	ISPAPB0_REG8(0x2725) = 0x07;
	ISPAPB0_REG8(0x2726) = 0x3C;	/* vsize */
	ISPAPB0_REG8(0x2727) = 0x04;
	ISPAPB0_REG8(0x275A) = 0x01;
	ISPAPB0_REG8(0x2759) = 0x00;
	ISPAPB0_REG8(0x2604) = 0x00;
	//ISPAPB0_REG8(0x2007) = 0x01;

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

/*
	@cdspSetTable_s
*/
void cdspSetTable_s(void)
{
	int i;

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	ISPAPB0_REG8(0x2008) = 0x00; //use memory clock for pixel clock, master clock and mipi decoder clock
	// R table of lens compensation tables
	ISPAPB0_REG8(0x2101) = 0x00; // select lens compensation R SRAM
	ISPAPB0_REG8(0x2100) = 0x03; // enable CPU access macro and adress auto increase
	ISPAPB0_REG8(0x2104) = 0x00; // select macro page 0
	ISPAPB0_REG8(0x2102) = 0x00; // set macro address to 0	
	for (i = 0; i < 768; i++)
	{
		ISPAPB0_REG8(0x2103) = SF_LENS_COMP_R_S[i];
	}
	//
	// G/Gr table of lens compensation tables
	ISPAPB0_REG8(0x2101) = 0x01; // select lens compensation G/Gr SRAM
	ISPAPB0_REG8(0x2100) = 0x03; // enable CPU access macro and adress auto increase
	ISPAPB0_REG8(0x2104) = 0x00; // select macro page 0
	ISPAPB0_REG8(0x2102) = 0x00; // set macro address to 0
	for (i = 0; i < 768; i++)
	{
		ISPAPB0_REG8(0x2103) = SF_LENS_COMP_G_S[i];
	}
	//
	// B table of lens compensation tables
	ISPAPB0_REG8(0x2101) = 0x02; // select lens compensation B SRAM
	ISPAPB0_REG8(0x2100) = 0x03; // enable CPU access macro and adress auto increase
	ISPAPB0_REG8(0x2104) = 0x00; // select macro page 0
	ISPAPB0_REG8(0x2102) = 0x00; // set macro address to 0
	for (i = 0; i < 768; i++)
	{
		ISPAPB0_REG8(0x2103) = SF_LENS_COMP_B_S[i];
	}
	//
	/* write post gamma tables */
	// R table of post gamma tables
	ISPAPB0_REG8(0x2101) = 0x04; // select post gamma R SRAM
	ISPAPB0_REG8(0x2100) = 0x03; // enable CPU access macro and adress auto increase
	ISPAPB0_REG8(0x2104) = 0x00; // select macro page 0
	ISPAPB0_REG8(0x2102) = 0x00; // set macro address to 0
	for (i = 0; i < 512; i++)
	{
		ISPAPB0_REG8(0x2103) = SF_POST_GAMMA_R_S[i];
	}
	//
	// G table of post gamma tables
	ISPAPB0_REG8(0x2101) = 0x05; // select post gamma G SRAM
	ISPAPB0_REG8(0x2100) = 0x03; // enable CPU access macro and adress auto increase
	ISPAPB0_REG8(0x2104) = 0x00; // select macro page 0
	ISPAPB0_REG8(0x2102) = 0x00; // set macro address to 0
	for (i = 0; i < 512; i++)
	{
		ISPAPB0_REG8(0x2103) = SF_POST_GAMMA_G_S[i];
	}
	//
	// B table of of post gamma tables
	ISPAPB0_REG8(0x2101) = 0x06; // select post gamma B SRAM
	ISPAPB0_REG8(0x2100) = 0x03; // enable CPU access macro and adress auto increase
	ISPAPB0_REG8(0x2104) = 0x00; // select macro page 0
	ISPAPB0_REG8(0x2102) = 0x00; // set macro address to 0
	for (i = 0; i < 512; i++)
	{
		ISPAPB0_REG8(0x2103) = SF_POST_GAMMA_B_S[i];
	}
	//
	//  fixed pattern noise tables
	ISPAPB0_REG8(0x2101) = 0x0D; // select fixed pattern noise
	ISPAPB0_REG8(0x2100) = 0x03; // enable CPU access macro and adress auto increase
	ISPAPB0_REG8(0x2104) = 0x00; // select macro page 0
	ISPAPB0_REG8(0x2102) = 0x00; // set macro address to 0
	//for (i = 0; i < 1952; i++)
	for (i = 0; i < 1312; i++)
	{
		ISPAPB0_REG8(0x2103) = SF_FIXED_PATTERN_NOISE_S[i];
	}
	// disable set cdsp sram
	ISPAPB0_REG8(0x2104) = 0x00; // select macro page 0 
	ISPAPB0_REG8(0x2102) = 0x00; // set macro address to 0 
	ISPAPB0_REG8(0x2100) = 0x00; // disable CPU access macro and adress auto increase 

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

/*
	@sensorInit_s
*/
void sensorInit_s(void)
{
	//SENSOR_INIT_FILE_T *read_file = (SENSOR_INIT_FILE_T *)SENSOR_INIT_FILE;
	SENSOR_INIT_FILE_T read_file;
	int i, total;
	u8 data[4];

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

 	// conver table data to SENSOR_INIT_FILE_T struct
	total = ARRAY_SIZE(SENSOR_INIT_FILE);
	data[0] = SENSOR_INIT_FILE[0];
	data[1] = SENSOR_INIT_FILE[1];
	data[2] = SENSOR_INIT_FILE[2];
	data[3] = SENSOR_INIT_FILE[3];
	read_file.count = (data[0]<<8)|data[1];

	ISPAPB_LOGI("%s, total=%d\n", __FUNCTION__, total);
	ISPAPB_LOGI("%s, count=%d, data[0]=0x%02x, data[1]=0x%02x, data[2]=0x%02x, data[3]=0x%02x\n", __FUNCTION__, read_file.count, data[0], data[1], data[2], data[3]);
	for (i = 2; i < total; i=i+4)
	{
		data[0] = SENSOR_INIT_FILE[i];
		data[1] = SENSOR_INIT_FILE[i+1];
		data[2] = SENSOR_INIT_FILE[i+2];
		data[3] = SENSOR_INIT_FILE[i+3];
		read_file.SENSOR_DATA[(i-2)/4].type = data[0];
		read_file.SENSOR_DATA[(i-2)/4].adr  = (data[1]<<8)|data[2];
		read_file.SENSOR_DATA[(i-2)/4].dat  = data[3];
		//ISPAPB_LOGI("%s, i=%d, data[0]=0x%02x, data[1]=0x%02x, data[2]=0x%02x, data[3]=0x%02x\n", __FUNCTION__, i, data[0], data[1], data[2], data[3]);
	}

	I2CReset();
	I2CInit(SC2310_DEVICE_ADDR, 0);

	ISPAPB_LOGI("%s, count=%d\n", __FUNCTION__, read_file.count);
	for (i = 0; i < read_file.count; i++)
	{
		//ISPAPB_LOGI("%s, type=0x%02x, adr=0x%04x, dat=0x%04x\n", __FUNCTION__, read_file.SENSOR_DATA[i].type, read_file.SENSOR_DATA[i].adr, read_file.SENSOR_DATA[i].dat);

		if (read_file.SENSOR_DATA[i].type == 0xFE)
		{
			udelay(read_file.SENSOR_DATA[i].adr*1000);
		}
		else if (read_file.SENSOR_DATA[i].type == 0x00)
		{
			setSensor16_I2C1((unsigned long)read_file.SENSOR_DATA[i].adr, read_file.SENSOR_DATA[i].dat, 1);
		}
	}

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

/*
	@CdspInit_s
*/
void CdspInit_s(void)
{	
	//CDSP_INIT_FILE_T *read_file = (CDSP_INIT_FILE_T *)CDSP_INIT_FILE;
	CDSP_INIT_FILE_T read_file;
	int i, total;
	u8 data[4];

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

 	// conver table data to CDSP_INIT_FILE_T struct
	total = ARRAY_SIZE(CDSP_INIT_FILE);
	data[0] = CDSP_INIT_FILE[0];
	data[1] = CDSP_INIT_FILE[1];
	data[2] = CDSP_INIT_FILE[2];
	data[3] = CDSP_INIT_FILE[3];
	read_file.count = (data[0]<<8)|data[1];

	ISPAPB_LOGI("%s, total=%d\n", __FUNCTION__, total);
	ISPAPB_LOGI("%s, count=%d, data[0]=0x%02x, data[1]=0x%02x, data[2]=0x%02x, data[3]=0x%02x\n", __FUNCTION__, read_file.count, data[0], data[1], data[2], data[3]);
	for (i = 2; i < total; i=i+4)
	{
		data[0] = CDSP_INIT_FILE[i];
		data[1] = CDSP_INIT_FILE[i+1];
		data[2] = CDSP_INIT_FILE[i+2];
		data[3] = CDSP_INIT_FILE[i+3];
		read_file.CDSP_DATA[(i-2)/4].type = data[0];
		read_file.CDSP_DATA[(i-2)/4].adr  = (data[1]<<8)|data[2];
		read_file.CDSP_DATA[(i-2)/4].dat  = data[3];
		//ISPAPB_LOGI("%s, i=%d, data[0]=0x%02x, data[1]=0x%02x, data[2]=0x%02x, data[3]=0x%02x\n", __FUNCTION__, i, data[0], data[1], data[2], data[3]);
	}

	//clock setting	
	ISPAPB0_REG8(0x21d0) = 0x01; //sofware reset CDSP interface (active)
	ISPAPB1_REG8(0x21d0) = 0x00; //sofware reset CDSP interface (inactive)

	ISPAPB_LOGI("%s, count=%d\n", __FUNCTION__, read_file.count);
	for (i = 0; i<read_file.count; i++)
	{
		//ISPAPB_LOGI("%s, type=0x%02x, adr=0x%04x, dat=0x%04x\n", __FUNCTION__, read_file.CDSP_DATA[i].type, read_file.CDSP_DATA[i].adr, read_file.CDSP_DATA[i].dat);

		if (read_file.CDSP_DATA[i].type == 0x00)
		{
			ISPAPB0_REG8((unsigned long)read_file.CDSP_DATA[i].adr) = read_file.CDSP_DATA[i].dat;
		}
	}
	//
	ISPAPB0_REG8(0x220C) = 0x33; //SF_CDSP_INIT_SM
	ISPAPB0_REG8(0x21c0) = 0x23; /* enable bight, contrast hue and saturation cropping mode */
/*	                             
	#if (CDSP_SCALER_HD)//scale down from FHD  to HD size
		//H=1280*65536/(1920) = 0xAAAB
	   //V=720*65536/(1080) = 0xAAAB
		ISPAPB0_REG8(0x21b0) = 0xAB;//factor for Hsize
		ISPAPB0_REG8(0x21b1) = 0xAA;
		ISPAPB0_REG8(0x21b2) = 0xAB;//factor for Vsize
		ISPAPB0_REG8(0x21b3) = 0xAA;
		//
		ISPAPB0_REG8(0x21b4) = 0xAB;//factor for Hsize
		ISPAPB0_REG8(0x21b5) = 0xAA;
		ISPAPB0_REG8(0x21b6) = 0xAB;//factor for Vsize
		ISPAPB0_REG8(0x21b7) = 0xAA;
		//
		ISPAPB0_REG8(0x21b8) = 0x2F;	//enable	
	#elif (CDSP_SCALER_VGA)//scale down from FHD to VGA size	
	   //H=640*65536/(1920) = 0x5556
	   //V=480*65536/(1080) = 0x71C8
		ISPAPB0_REG8(0x21b0) = 0x56;//factor for Hsize
		ISPAPB0_REG8(0x21b1) = 0x55;
		ISPAPB0_REG8(0x21b2) = 0xC8;//factor for Vsize
		ISPAPB0_REG8(0x21b3) = 0x71;
		//
		ISPAPB0_REG8(0x21b4) = 0x56;//factor for Hsize
		ISPAPB0_REG8(0x21b5) = 0x55;
		ISPAPB0_REG8(0x21b6) = 0xC8;//factor for Vsize
		ISPAPB0_REG8(0x21b7) = 0x71;
		//
		ISPAPB0_REG8(0x21b8) = 0x2F;	//enable	
	#elif (CDSP_SCALER_QQVGA)//scale down from FHD  to QVGA size	     
		//H=160*65536/(1920) = 0x1556
	    //V=120*65536/(1080) = 0x1C72
		ISPAPB0_REG8(0x21b0) = 0x56;//factor for Hsize
		ISPAPB0_REG8(0x21b1) = 0x15;
		ISPAPB0_REG8(0x21b2) = 0x72;//factor for Vsize
		ISPAPB0_REG8(0x21b3) = 0x1C;
		//
		ISPAPB0_REG8(0x21b4) = 0x56;//factor for Hsize
		ISPAPB0_REG8(0x21b5) = 0x15;
		ISPAPB0_REG8(0x21b6) = 0x72;//factor for Vsize
		ISPAPB0_REG8(0x21b7) = 0x1C;
		//
		ISPAPB0_REG8(0x21b8) = 0x2F;	//enable
	#else	
		ISPAPB0_REG8(0x21b8) = 0x00; //disable H/V scale down                              
	#endif                           
*/
	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

/*
	@ispAaaInit_s
*/
void ispAaaInit_s(void)
{
	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	//not ready

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

void videoStartMode(void)
{
	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	powerSensorOn_RAM();
	FrontInit_s(1920, 1080);
	CdspInit_s();
	ispAaaInit_s();
	sensorInit_s();

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

void videoStopMode(void)
{
	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	//not ready
	//powerSensorDown_RAM();

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}

void isp_setting_s(void)
{
	int i;

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	ispReset_s();
	cdspSetTable_s();
	
	for(i=0; i<1; i++)
	{
		videoStartMode();
		//sleep(20);
		videoStopMode();
		//sleep(20);
	}

	ISPAPB_LOGI("%s end\n", __FUNCTION__);
}
