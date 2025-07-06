#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "PWM.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void USART2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART2, ENABLE);
}
void parse_and_control_servo(void)
{
    char buffer[50];
    uint8_t i = 0;
    char ch;

    while (1)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
        ch = USART_ReceiveData(USART2);

        if (ch == '\n')
        {
            buffer[i] = '\0';
            OLED_ShowString(2, 1, "Recv:");
            OLED_ShowString(2, 7, buffer);

            char *ax_ptr = strstr(buffer, "AX:");
            char *ay_ptr = strstr(buffer, "AY:");

            if (ax_ptr && ay_ptr)
            {
                int ax = atoi(ax_ptr + 3);
                int ay = atoi(ay_ptr + 3);
				if (ax >= -90 && ax <= 90)
				{
                int pwm_ax = 1500 + ax * 10;
                if (pwm_ax < 800) pwm_ax = 800;
                if (pwm_ax > 2200) pwm_ax = 2200;
                PWM_SetCompare2(pwm_ax);
               // 清除旧数据
               OLED_ShowString(3, 1, "AX:");     // 固定标签位置
               OLED_ShowSignedNum(3, 5, ax, 3);  // 数值从第5列开始显示3位
				}
				 if (ay >= -90 && ay <= 90)
				{
                int pwm_ay = 1500 + ay * 10;
                if (pwm_ay < 800) pwm_ay = 800;
                if (pwm_ay > 2200) pwm_ay = 2200;
                PWM_SetCompare3(pwm_ay);
                
                OLED_ShowString(4, 1, "AY:");
                OLED_ShowSignedNum(4, 5, ay, 3);
				}
            }

            i = 0;
        }
        else
        {
            if (i < sizeof(buffer) - 1)
                buffer[i++] = ch;
        }
    }
}

int main(void)
{
    OLED_Init();
    PWM_Init();
    USART2_Init();

    OLED_ShowString(1, 1, "Dual Servo RX");

    while (1)
    {
        parse_and_control_servo();
    }
}
