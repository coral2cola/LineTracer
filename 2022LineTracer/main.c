 #include "msp.h"
#include "..\inc\Clock.h"
#include "..\inc\SysTick.h"
#include <stdio.h>

void (*TimerA0Task) (void);
int stack[100];
int top = 0;
int pop = 0;
int direction = -1;


void TimerA0_Init(void(*task)(void), uint16_t period) {
   TimerA0Task = task;
   TIMER_A0->CTL = 0x0280;
   TIMER_A0->CCTL[0] = 0x0010;
   TIMER_A0->CCR[0] = (period -1);
   TIMER_A0->EX0 = 0x0005;
   NVIC->IP[3] = (NVIC->IP[3]&0xFFFFFF00) | 0x00000040;
   NVIC->ISER[0] = 0x00001000;
   TIMER_A0->CTL |= 0x0014;
}



void TA2_0_IRQHandler(void) {
   TIMER_A0->CCTL[0] &= ~0x0001;
   (*TimerA0Task)();
}

void PWM_Init34(uint16_t period, uint16_t duty3, uint16_t duty4)
{
    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;

    TIMER_A0->CCTL[0] = 0x800;
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;
}

void Motor_Init(){
   P3->SEL0 &= ~0xC0;
   P3->SEL1 &= ~0xC0;
   P3->DIR |= 0xC0;
   P3->OUT &= ~0xC0;

   P5->SEL0 &= ~0x30;
   P5->SEL1 &= ~0x30;
   P5->DIR |= 0x30;
   P5->OUT &= ~0x30;

   P2->SEL0 &= ~0xC0;
   P2->SEL1 &= ~0xC0;
   P2->DIR |= 0xC0;
   P2->OUT &= ~0xC0;

   PWM_Init34(15000, 0, 0);
}


void Left_Forward() {
   P5->OUT &= ~0x10;
}

void Left_Backward() {
   P5->OUT |= 0x10;
}
void Right_Forward() {
   P5->OUT &= ~0x20;
}

void Right_Backward() {
    P5->OUT |= 0x20;
}

void PWM_Duty3(uint16_t duty3) {
   TIMER_A0->CCR[3] = duty3;
}

void PWM_Duty4(uint16_t duty4)
{
   TIMER_A0->CCR[4] = duty4;
}

void Move(uint16_t leftDuty, uint16_t rightDuty) {
   P3->OUT |= 0xC0;
   PWM_Duty3(rightDuty);
   PWM_Duty4(leftDuty);
}

void task() {
   printf("interrupt occurs!\n");
}

void Go(int direction)
{
    if(direction == 0) // left
    {
        Left_Forward();
        Right_Forward();
        Move(950, 960);
        printf("straight\n");
        Clock_Delay1ms(100);

        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;

        Right_Forward();
        Left_Backward();
        Move(1650, 1645);
        printf("left\n");
        Clock_Delay1ms(1500);

    }
    else if(direction == 1) // right
    {
        Left_Forward();
        Right_Forward();
        Move(950, 960);
        printf("straight\n");
        Clock_Delay1ms(100);

        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;

        Left_Forward();
        Right_Backward();
        Move(1645, 1650);
        printf("right\n");
        //Clock_Delay1ms(1500);
    }
    else if(direction == 2) // straight
    {
        Left_Forward();
        Right_Forward();
        Move(950, 955);
        printf("stra\n");
        Clock_Delay1ms(10);
    }
    else if(direction == 3) // stop
    {
        Left_Forward();
        Right_Forward();
        Move(0, 0);
        printf("stop\n");
        Clock_Delay1ms(10);
    }
    else if(direction == 4) // back
    {
        Left_Forward();
        Right_Backward();
        Move(1650, 1650);
        Clock_Delay1ms(3600);
    }
    else // error
    {
        printf("ERROR!\n");
    }

}

void Push(int direction)
{
    stack[top] = direction;
    top += 1;
}

// left = 0;
// right = 1;
// straight = 2;
int Pop()
{
    direction = stack[top];
    top -= 1;
    pop = 0;

    if(direction == 1)
    {
        direction = 0; // Left
    }
    else if(direction == 2 || direction == 3)
    {
        direction = 1; // Right
    }
    else if(direction == 4)
    {
        direction = 2; // Straight
    }
    else if(direction == 33)
    {
        direction = 2; // Straight
        Push(3);
    }
    else if(direction == 44)
    {
        direction = 1; // Right
        Push(4);
    }
    return direction;
}


/**
 * main.c
 */
void main(void)
{
    Clock_Init48MHz();
    Motor_Init();

    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR &= ~0xFF;


    int cnt = 0;


    while(1)
    {
        P5->OUT |= 0x08;
        P9->OUT |= 0x04;

        P7->DIR = 0xFF;
        P7->OUT = 0xFF;

        Clock_Delay1us(10);

        P7->DIR = 0x00;

        Clock_Delay1us(1000);

        int sensor_start, sensor_stop, sensor_straight, sensor_right, sensor_left;

        sensor_start = P7->IN & 0x7E;
        sensor_stop = P7->IN & 0x3C;
        sensor_straight = P7->IN & 0x18;
        sensor_right = P7->IN & 0x1F;
        sensor_left = P7->IN & 0xF8;

        if(sensor_start == 0x7E)
        {
            // Left_Forward();
            // Right_Forward();
            // Move(1975, 1980);
            // printf("start\n");
            // Clock_Delay1ms(10);
            Go(2);
            printf("START\n");
        }
        else if(sensor_stop == 0x3C )
        {
            // Left_Forward();
            // Right_Forward();
            // Move(0, 0);
            // printf("stop\n");
            // Clock_Delay1ms(10);
            Go(3);
            printf("STOP\n");
            break;
        }
        else if(sensor_straight == 0x18)
        {
            // Left_Forward();
            // Right_Forward();
            // Move(1975, 1980);
            // printf("stra\n");
            // Clock_Delay1ms(10);
            Go(2);
            printf("FORWARD\n");
        }
        else if(sensor_straight == 0x08)
        {
            Left_Forward();
            Right_Backward();
            Move(1975, 1980);
            printf("R_FIX\n");
            Clock_Delay1ms(50);
        }
        else if(sensor_straight == 0x10)
        {
            Right_Forward();
            Left_Backward();
            Move(1975, 1980);
            printf("L_FIX\n");
            Clock_Delay1ms(50);
        }
        else if(sensor_right & 0x06)
        {
            if(cnt < 5)
            {
                Go(2);
                printf("PROTECT\n");
            }
            else
            {
                Go(1);
                printf("RIGHT\n");
            }
        }
        else if(sensor_left & 0x60)
        {
            if(cnt < 5)
            {
                Go(2);
                printf("PROTECT\n");
            }
            else
            {
                Go(0);
                printf("LEFT\n");
            }
        }
        else // Intersection or Blank space
        {
            if(pop == 1)
            {
                // direction = stack[top];
                // top -= 1;
                // pop = 0;
                direction = Pop();
                Go(direction);
            }
            else if(pop == 0)
            {
                if(sensor_left & 0xC0) // |-
                {
                    // stack[top] = 0;
                    // top += 1;
                    Push(1);
                }
                else if(sensor_right & 0x03) // -|
                {
                    Push(2);
                }
                else if(sensor_left==0xF8 && sensor_right==0x1F) // T
                {
                    Push(33);
                }
                else if(sensor_straight==0x18 && sensor_left==0xF8 && sensor_right==0x1F ) // +
                {
                    Push(44);
                }
                else
                {
                    pop = 1;
                    Go(4);
                    printf("turn around\n");
                }
            }
        }



        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;

        Clock_Delay1ms(10);

        cnt ++;
    }
}
