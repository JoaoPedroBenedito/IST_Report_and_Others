#include "xparameters.h"
#include "xgpio.h"
#include "fsl.h"
#include "math.h"

#define MY_GPIO_ID XPAR_MICROBLAZE_ID   // macro that gets my GPIO ID from the parameters file xparameters.h
#define LED_DELAY 100000             // This is a delay between updates in the leds (in microseconds)
#define LED_CHANNEL 1               // used to determine which channel of the GPIO is used
#define GPIO_BITWIDTH 8              // This is the width we set for the GPIO
#define camerain_slot_id 0              // FSL related macro
#define cameraout_slot_id 0             // FSL related macro
#define num_lin (64)                 // number of lines in the frame
#define num_col (128)               // number of columns in the frame
#define pixel(a,b) (image[a * num_lin + b])       // pixel address macro
#define N (num_lin*num_col)             // number of pixels in a frame

unsigned char image[N];               //image buffer
unsigned int vec_h[num_col];				//this is important and has to be changed in previous work
//short int img_emb[N];

/*void negative(){
	int i;

	for (i = 0; i < N; i++){
		image[i] = 255 - image[i];
	}
	return;
}

void histo(){
	short int vec_h[num_col];
	int i, j, max = 0, aux, height;

	for(i = 0; i < num_col; i++)
		vec_h[i] = 0;

	for(i = 0; i < N; i++)
		vec_h[image[i] / (256 / num_col)]++;

	for(i = 0; i < num_col; i++)
		max = (vec_h[i] > max) ? vec_h[i] : max;

	for(i = 0; i < num_col; i++) // select column
	{
		height = (vec_h[i] * num_lin) / max;

		for(j = 0; j < num_lin; j++) // select line
		{
			aux = j * num_col + i;

			image[aux] = ((num_lin - j) < height) ? 0 : 255;
		}
	}
	return;
}

void emboss2(){
	int i, aux;
	int k[] = {-2, -2, 0, -2, 12, -2, 0, -2, -2};

	for(i = 0; i < N; i++)
	{
		if(!(i < num_col || i > (N - num_col) || (i % num_col) == 0 || (i % num_col) == (num_col - 1)))
		{
			aux = 0;

			aux += k[0] * image[i + num_col - 1];
			aux += k[1] * image[i + num_col];
			aux += k[2] * image[i + num_col + 1];

			aux += k[3] * image[i - 1];
			aux += k[4] * image[i];
			aux += k[5] * image[i + 1];

			aux += k[6] * image[i - num_col - 1];
			aux += k[7] * image[i - num_col];
			aux += k[8] * image[i - num_col + 1];

			aux = (aux * 0.3) + 128;

			image[i - num_col - 1] = aux;
		}
	}
}

void emboss(){
	int i, aux, max_abs = 0;
	int k[] = {-2, -2, 0, -2, 12, -2, 0, -2, -2};

	for(i = 0; i < N; i++)
	{
		img_emb[i] = 0;

		if(!(i < num_col || i > (N - num_col) || (i % num_col) == 0 || (i % num_col) == (num_col - 1)))
		{
			img_emb[i] += k[0] * image[i + num_col - 1];
			img_emb[i] += k[1] * image[i + num_col];
			img_emb[i] += k[2] * image[i + num_col + 1];

			img_emb[i] += k[3] * image[i - 1];
			img_emb[i] += k[4] * image[i];
			img_emb[i] += k[5] * image[i + 1];

			img_emb[i] += k[6] * image[i - num_col - 1];
			img_emb[i] += k[7] * image[i - num_col];
			img_emb[i] += k[8] * image[i - num_col + 1];

			aux = (img_emb[i] > 0) ? img_emb[i] : -img_emb[i];

			max_abs = (aux > max_abs) ? aux : max_abs;
		}
	}

	for(i = 0; i < N; i++)
	{
		if(!(i < num_col || i > (N - num_col) || (i % num_col) == 0 || (i % num_col) == (num_col - 1)))
		{
			img_emb[i] = (img_emb[i] * 128) / max_abs;
			image[i] = img_emb[i] + 128;
		}
	}
}*/

void histo_vhdl(){
	int i, j, max = 0, aux, height;

	for(i = 0; i < num_col; i++)
		max = (vec_h[i] > max) ? vec_h[i] : max;

	for(i = 0; i < num_col; i++) // select column
	{
		height = (vec_h[i] * num_lin) / max;

		for(j = 0; j < num_lin; j++) // select line
		{
			aux = j * num_col + i;

			image[aux] = ((num_lin - j) < height) ? 0 : 255;
		}
	}
	return;
}

int main(void)
{
	unsigned int OutData = 0x01;     // will contain the value presented in the leds
	XGpio GpioDrv;           // this is the driver for our GPIO
	int DelayCtl;             // delay control variable
	int i;               // pixel index
	XGpio_Initialize(&GpioDrv, MY_GPIO_ID);                         // Initialize the GPIO driver
	XGpio_SetDataDirection(&GpioDrv, LED_CHANNEL, 0x0);                   // Set the direction for all signals to be outputs
	while(1)                                       // repeat forever:
	{
		if(OutData & 0x80) OutData = 0x01; else OutData <<= 1;                   // shift the current on‐led to the left
		XGpio_DiscreteWrite(&GpioDrv, LED_CHANNEL, OutData);                   // write to the correct channel of the GPIO
		for(DelayCtl = 0; DelayCtl < LED_DELAY; DelayCtl++);                     // apply the delay before next update
		//for (i = 0; i < N; i++) microblaze_bread_datafsl(image[i], cameraout_slot_id);           // read a full frame from the camera
		for (i = 0; i < num_col; i++) microblaze_bread_datafsl(vec_h[i], cameraout_slot_id);
		histo_vhdl();
		for (i = 0; i < N; i++) microblaze_bwrite_datafsl(image[i], camerain_slot_id);           // write a full frame to the VGA output
	}
	return 0;                                      // end application
}