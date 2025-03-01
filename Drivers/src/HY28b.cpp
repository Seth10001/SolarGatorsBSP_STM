/*
 * LCD.c
 *
 *  Created on: Jan 19, 2021
 *      Author: John Carr
 */


#include <HY28b.hpp>
#include "AsciiLib.h"
#include "cmsis_os.h"

/************************************  Private Functions  *******************************************/

/*
 * Delay x ms
 */
static void Delay(unsigned long interval)
{
  osDelay(interval);
}
HY28b::~HY28b() {
}
/*******************************************************************************
 * Function Name  : LCD_reset
 * Description    : Resets LCD
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : Uses P10.0 for reset
 *******************************************************************************/
static void LCD_reset()
{
	  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    Delay(100);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
    Delay(100);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
}

/************************************  Private Functions  *******************************************/


/************************************  Public Functions  *******************************************/

/*******************************************************************************
 * Function Name  : LCD_DrawRectangle
 * Description    : Draw a rectangle as the specified color
 * Input          : xStart, xEnd, yStart, yEnd, Color
 * Output         : None
 * Return         : None
 * Attention      : Must draw from left to right, top to bottom!
 *******************************************************************************/
inline void HY28b::DrawRectangle(int16_t xStart, int16_t xEnd, int16_t yStart, int16_t yEnd, uint16_t Color)
{
    WriteReg(HOR_ADDR_START_POS, yStart);
    WriteReg(HOR_ADDR_END_POS, yEnd);
	WriteReg(VERT_ADDR_START_POS, xStart);
	WriteReg(VERT_ADDR_END_POS, xEnd);
    SetCursor(xStart, yStart);
    WriteIndex(GRAM);
    SPI_CS_LOW;
    WriteDataStart();
    int total = (xEnd - xStart + 1)*(yEnd - yStart + 1);
    for (int i = 0; i < total; ++i)
    {
        WriteDataOnly(Color);
    }
    SPI_CS_HIGH;
}

/******************************************************************************
 * Function Name  : PutChar
 * Description    : Lcd screen displays a character
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - ASCI: Displayed character
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void HY28b::PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor)
{
    uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* get font data */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( ((tmp_char >> (7 - j)) & 0x01) == 0x01 )
            {
              if(size_ == 1)
                SetPoint( Xpos + j, Ypos + i, charColor );  /* Character color */
              else
              {
                DrawRectangle(Xpos + j * size_, Xpos + j * size_ + size_, Ypos + i * size_, Ypos + i * size_ + size_, charColor);
              }
            }
        }
    }
}

/******************************************************************************
 * Function Name  : GUI_Text
 * Description    : Displays the string
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - str: Displayed string
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void HY28b::DrawText(uint16_t Xpos, uint16_t Ypos, uint8_t *str, uint16_t Color)
{
    uint8_t TempChar;

    /* Set area back to span the entire LCD */
    WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */
    do
    {
        TempChar = *str++;
        PutChar( Xpos, Ypos, TempChar, Color);
        if( Xpos < MAX_SCREEN_X - 8)
        {
            Xpos += (8 * size_);
        }
        else if ( Ypos < MAX_SCREEN_X - 16)
        {
            Xpos = 0;
            Ypos += (16 * size_);
        }
        else
        {
            Xpos = 0;
            Ypos = 0;
        }
    }
    while ( *str != 0 );
}


/*******************************************************************************
 * Function Name  : LCD_Clear
 * Description    : Fill the screen as the specified color
 * Input          : - Color: Screen Color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void HY28b::Clear(uint16_t Color)
{
    DrawRectangle(0, MAX_SCREEN_X, 0, MAX_SCREEN_Y, Color);
}

/******************************************************************************
 * Function Name  : LCD_SetPoint
 * Description    : Drawn at a specified point coordinates
 * Input          : - Xpos: Row Coordinate
 *                  - Ypos: Line Coordinate
 * Output         : None
 * Return         : None
 * Attention      : 18N Bytes Written
 *******************************************************************************/
void HY28b::SetPoint(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
    SetCursor(Xpos, Ypos);
    WriteReg(GRAM, color);
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Only
 * Description    : Data writing to the LCD controller
 * Input          : - data: data to be written
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void HY28b::WriteDataOnly(uint16_t data)
{
    SPISendRecvByte((data >>   8));                    /* Write D8..D15                */
    SPISendRecvByte((data & 0xFF));                    /* Write D0..D7                 */
}

/*******************************************************************************
 * Function Name  : LCD_WriteData
 * Description    : LCD write register data
 * Input          : - data: register data
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void HY28b::WriteData(uint16_t data)
{
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
    SPISendRecvByte((data >>   8));                    /* Write D8..D15                */
    SPISendRecvByte((data & 0xFF));                    /* Write D0..D7                 */

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Reads the selected LCD Register.
 * Input          : None
 * Output         : None
 * Return         : LCD Register Value.
 * Attention      : None
 *******************************************************************************/
uint16_t HY28b::ReadReg(uint16_t LCD_Reg)
{
    WriteIndex(LCD_Reg);
    return ReadData();
}

/*******************************************************************************
 * Function Name  : LCD_WriteIndex
 * Description    : LCD write register address
 * Input          : - index: register address
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void HY28b::WriteIndex(uint16_t index)
{
    SPI_CS_LOW;

    /* SPI write data */
    SPISendRecvByte(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0  */
    SPISendRecvByte(0);
    SPISendRecvByte(index);

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : SPISendRecvByte
 * Description    : Send one byte then receive one byte of response
 * Input          : uint8_t: byte
 * Output         : None
 * Return         : Recieved value
 * Attention      : None
 *******************************************************************************/
inline uint8_t HY28b::SPISendRecvByte (uint8_t byte)
{
	uint8_t rxData = 0;
    HAL_SPI_TransmitReceive(this->spi, &byte, &rxData, 1, HAL_MAX_DELAY);
    return rxData;
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Start
 * Description    : Start of data writing to the LCD controller
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void HY28b::WriteDataStart(void)
{
    SPISendRecvByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0 */
}

/*******************************************************************************
 * Function Name  : LCD_ReadData
 * Description    : LCD read data
 * Input          : None
 * Output         : None
 * Return         : return data
 * Attention      : Diagram (d) in datasheet
 *******************************************************************************/
inline uint16_t HY28b::ReadData()
{
    uint16_t value;
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_RD | SPI_DATA);   /* Read: RS = 1, RW = 1   */
    SPISendRecvByte(0);                               /* Dummy read 1           */
    value = (SPISendRecvByte(0) << 8);                /* Read D8..D15           */
    value |= SPISendRecvByte(0);                      /* Read D0..D7            */

    SPI_CS_HIGH;
    return value;
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Writes to the selected LCD register.
 * Input          : - LCD_Reg: address of the selected register.
 *                  - LCD_RegValue: value to write to the selected register.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void HY28b::WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
    WriteIndex(LCD_Reg);
    WriteData(LCD_RegValue);
}

/*******************************************************************************
 * Function Name  : LCD_SetCursor
 * Description    : Sets the cursor position.
 * Input          : - Xpos: specifies the X position.
 *                  - Ypos: specifies the Y position.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void HY28b::SetCursor(uint16_t Xpos, uint16_t Ypos )
{
    WriteReg(GRAM_HORIZONTAL_ADDRESS_SET, Ypos);
    WriteReg(GRAM_VERTICAL_ADDRESS_SET, Xpos);
}

/*******************************************************************************
 * Function Name  : LCD_Init
 * Description    : Configures LCD Control lines, sets whole screen black
 * Input          : bool usingTP: determines whether or not to enable TP interrupt
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
HY28b::HY28b(SPI_HandleTypeDef* hspi, bool usingTP):spi{hspi}
{
  size_ = 1;
	SPI_CS_HIGH;
	SPI_CS_TP_HIGH;
    if (usingTP)
    {
        // port 4 interrupt setup
//        P4->DIR &= ~BIT0;
//        P4->IFG &= ~BIT0;
//        P4->IE |= BIT0;
//        P4->IES |= BIT0;
    }

    LCD_reset();

    WriteReg(0xE5, 0x78F0);                             /* set SRAM internal timing */
    WriteReg(DRIVER_OUTPUT_CONTROL, 0x0100);            /* set Driver Output Control */
    WriteReg(DRIVING_WAVE_CONTROL, 0x0700);             /* set 1 line inversion */
    WriteReg(ENTRY_MODE, 0x1038);                       /* set GRAM write direction and BGR=1 */
    WriteReg(RESIZING_CONTROL, 0x0000);                 /* Resize register */
    WriteReg(DISPLAY_CONTROL_2, 0x0207);                /* set the back porch and front porch */
    WriteReg(DISPLAY_CONTROL_3, 0x0000);                /* set non-display area refresh cycle ISC[3:0] */
    WriteReg(DISPLAY_CONTROL_4, 0x0000);                /* FMARK function */
    WriteReg(RGB_DISPLAY_INTERFACE_CONTROL_1, 0x0000);  /* RGB interface setting */
    WriteReg(FRAME_MARKER_POSITION, 0x0000);            /* Frame marker Position */
    WriteReg(RGB_DISPLAY_INTERFACE_CONTROL_2, 0x0000);  /* RGB interface polarity */

    /* Power On sequence */
    WriteReg(POWER_CONTROL_1, 0x0000);        /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    WriteReg(POWER_CONTROL_2, 0x0007);        /* DC1[2:0], DC0[2:0], VC[2:0] */
    WriteReg(POWER_CONTROL_3, 0x0000);        /* VREG1OUT voltage */
    WriteReg(POWER_CONTROL_4, 0x0000);        /* VDV[4:0] for VCOM amplitude */
    WriteReg(DISPLAY_CONTROL_1, 0x0001);
    Delay(200);

    /* Dis-charge capacitor power voltage */
    WriteReg(POWER_CONTROL_1, 0x1090);              /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    WriteReg(POWER_CONTROL_2, 0x0227);              /* Set DC1[2:0], DC0[2:0], VC[2:0] */
    Delay(50);                                      /* Delay 50ms */
    WriteReg(POWER_CONTROL_3, 0x001F);
    Delay(50);                                      /* Delay 50ms */
    WriteReg(POWER_CONTROL_4, 0x1500);              /* VDV[4:0] for VCOM amplitude */
    WriteReg(POWER_CONTROL_7, 0x0027);              /* 04 VCM[5:0] for VCOMH */
    WriteReg(FRAME_RATE_AND_COLOR_CONTROL, 0x000D); /* Set Frame Rate */
    Delay(50);                                      /* Delay 50ms */
    WriteReg(GRAM_HORIZONTAL_ADDRESS_SET, 0x0000);  /* GRAM horizontal Address */
    WriteReg(GRAM_VERTICAL_ADDRESS_SET, 0x0000);    /* GRAM Vertical Address */

    /* Adjust the Gamma Curve */
    WriteReg(GAMMA_CONTROL_1,    0x0000);
    WriteReg(GAMMA_CONTROL_2,    0x0707);
    WriteReg(GAMMA_CONTROL_3,    0x0307);
    WriteReg(GAMMA_CONTROL_4,    0x0200);
    WriteReg(GAMMA_CONTROL_5,    0x0008);
    WriteReg(GAMMA_CONTROL_6,    0x0004);
    WriteReg(GAMMA_CONTROL_7,    0x0000);
    WriteReg(GAMMA_CONTROL_8,    0x0707);
    WriteReg(GAMMA_CONTROL_9,    0x0002);
    WriteReg(GAMMA_CONTROL_10,   0x1D04);

    /* Set GRAM area */
    WriteReg(HOR_ADDR_START_POS, 0x0000);             /* Horizontal GRAM Start Address */
    WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));   /* Horizontal GRAM End Address */
    WriteReg(VERT_ADDR_START_POS, 0x0000);            /* Vertical GRAM Start Address */
    WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1));  /* Vertical GRAM Start Address */
    WriteReg(GATE_SCAN_CONTROL_0X60, 0x2700);         /* Gate Scan Line */
    WriteReg(GATE_SCAN_CONTROL_0X61, 0x0001);         /* NDL,VLE, REV */
    WriteReg(GATE_SCAN_CONTROL_0X6A, 0x0000);         /* set scrolling line */

    /* Partial Display Control */
    WriteReg(PART_IMAGE_1_DISPLAY_POS, 0x0000);
    WriteReg(PART_IMG_1_START_END_ADDR_0x81, 0x0000);
    WriteReg(PART_IMG_1_START_END_ADDR_0x82, 0x0000);
    WriteReg(PART_IMAGE_2_DISPLAY_POS, 0x0000);
    WriteReg(PART_IMG_2_START_END_ADDR_0x84, 0x0000);
    WriteReg(PART_IMG_2_START_END_ADDR_0x85, 0x0000);

    /* Panel Control */
    WriteReg(PANEL_ITERFACE_CONTROL_1, 0x0010);
    WriteReg(PANEL_ITERFACE_CONTROL_2, 0x0600);
    WriteReg(DISPLAY_CONTROL_1, 0x0133); /* 262K color and display ON */
    Delay(50); /* delay 50 ms */

    Clear(HY28b::BLACK);
}
#define ABS(x) (x < 0 ? x * -1 : x)
/**
 * @brief This reads a register from the touchpad controller
 * @input control_reg - This should be either the control reg for x or y
 * @retval The adc value associated with the control reg specified
 */
inline uint16_t HY28b::TP_ReadReg(uint8_t control_reg)
{
    //Disable tp_irq
	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
    SPI_CS_TP_LOW;
    SPISendRecvByte(control_reg);
    uint16_t adcData = 0;
    adcData |= ((uint16_t)SPISendRecvByte(0) & 0x7F) << 5; //Upper 7 bits
    adcData |= (SPISendRecvByte(0) & 0xF8) >> 3; //Lower 5 bits
    SPI_CS_TP_HIGH;
    //Restore tp_irq state
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
    return adcData;
}

static uint16_t avgClosestVals(uint16_t x1, uint16_t x2, uint16_t x3)
{
    uint16_t delta1 = ABS(x1 - x2);
    uint16_t delta2 = ABS(x1 - x3);
    uint16_t delta3 = ABS(x2 - x3);

    if(delta1 <= delta2 && delta1 <= delta3)
    {
        return ((uint32_t)x1 + x2) >> 1;
    }else if(delta2 <= delta1 && delta2 <= delta3)
    {
        return ((uint32_t)x1 + x3) >> 1;
    }else{
        return ((uint32_t)x2 + x3) >> 1;
    }
}
/*******************************************************************************
 * Function Name  : TP_ReadXY
 * Description    : Obtain X and Y touch coordinates
 * Input          : None
 * Output         : None
 * Return         : Pointer to "Point" structure
 * Attention      : None
 *******************************************************************************/
point_t HY28b::TP_ReadXY()
{
    point_t touchPoint;
    //Read 3 ADC vals to take the average
    uint16_t xVals[3], yVals[3];
    for(uint8_t i = 0; i < 3; i++)
    {
        xVals[i] = TP_ReadReg(CHX);
        yVals[i] = TP_ReadReg(CHY);
    }
    //Take average
    touchPoint.x = (avgClosestVals(xVals[0], xVals[1], xVals[2]) * 0.08702085) - 34.72708829456931;
    touchPoint.y = (avgClosestVals(yVals[0], yVals[1], yVals[2]) * 0.06249109) - 16.061182348789544;
//    touchPoint.x = ((float)TP_ReadReg(CHX)/0xFFF)*(MAX_SCREEN_X - 1);
//    touchPoint.y = ((float)TP_ReadReg(CHY)/0xFFF)*(MAX_SCREEN_Y - 1);
    return touchPoint;
}

void HY28b::SetSize(uint8_t size)
{
  size_ = size;
}

/************************************  Public Functions  *******************************************/

