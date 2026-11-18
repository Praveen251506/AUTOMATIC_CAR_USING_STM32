#include "Header.h"
#include "FUNCTION.h"
#include "motor.h"

/* =========================
   MOTOR STATE VARIABLES
   ========================= */

static volatile uint16_t current_pwm = 0;
static volatile uint16_t target_pwm = 0;

static volatile uint8_t motor_direction = MOTOR_STOPPED;
static volatile uint8_t requested_direction = MOTOR_STOPPED;

/*
 * Maximum PWM allowed by safety system.
 *
 * Normal     -> 300
 * Tilt warn  -> lower value
 * Tilt danger-> 0
 */
static volatile uint16_t safety_pwm_limit = MOTOR_TARGET_PWM;

static volatile uint8_t direction_delay = 0;

#define DIRECTION_DELAY 20
#define MOTOR_STEP 5

/* =========================
   SET MOTOR DIRECTION
   ========================= */

static void set_motor_direction(uint8_t direction)
{
    switch (direction)
    {
        case MOTOR_FORWARD:

            GPIOB->ODR |=  (1 << MOTOR_IN1);
            GPIOB->ODR &= ~(1 << MOTOR_IN2);

            GPIOB->ODR |=  (1 << MOTOR_IN3);
            GPIOB->ODR &= ~(1 << MOTOR_IN4);

            TIM2->CCR1 = current_pwm;
            TIM2->CCR2 = current_pwm;

            break;


        case MOTOR_BACKWARD:

            GPIOB->ODR &= ~(1 << MOTOR_IN1);
            GPIOB->ODR |=  (1 << MOTOR_IN2);

            GPIOB->ODR &= ~(1 << MOTOR_IN3);
            GPIOB->ODR |=  (1 << MOTOR_IN4);

            TIM2->CCR1 = current_pwm;
            TIM2->CCR2 = current_pwm;

            break;


        case MOTOR_RIGHT:

            GPIOB->ODR |=  (1 << MOTOR_IN1);
            GPIOB->ODR &= ~(1 << MOTOR_IN2);

            GPIOB->ODR &= ~(1 << MOTOR_IN3);
            GPIOB->ODR |=  (1 << MOTOR_IN4);

            TIM2->CCR1 = current_pwm;
            TIM2->CCR2 = current_pwm;

            break;


        case MOTOR_LEFT:

            GPIOB->ODR &= ~(1 << MOTOR_IN1);
            GPIOB->ODR |=  (1 << MOTOR_IN2);

            GPIOB->ODR |=  (1 << MOTOR_IN3);
            GPIOB->ODR &= ~(1 << MOTOR_IN4);

            TIM2->CCR1 = current_pwm;
            TIM2->CCR2 = current_pwm;

            break;


        default:

            GPIOB->ODR &= ~(1 << MOTOR_IN1);
            GPIOB->ODR &= ~(1 << MOTOR_IN2);

            GPIOB->ODR &= ~(1 << MOTOR_IN3);
            GPIOB->ODR &= ~(1 << MOTOR_IN4);

            TIM2->CCR1 = 0;
            TIM2->CCR2 = 0;

            break;
    }
}


/* =========================
   MOTOR INITIALIZATION
   ========================= */

void motor_init(void)
{
    /* GPIOA clock */
    RCC->AHB1ENR |= (1 << 0);

    /* GPIOB clock */
    RCC->AHB1ENR |= (1 << 1);


    /* =========================
       PA0 -> TIM2_CH1
       PA1 -> TIM2_CH2
       ========================= */

    GPIOA->MODER &= ~(3 << 0);
    GPIOA->MODER |=  (2 << 0);

    GPIOA->MODER &= ~(3 << 2);
    GPIOA->MODER |=  (2 << 2);


    /* PA0 / PA1 -> AF1 */

    GPIOA->AFRL &= ~(0xF << 0);
    GPIOA->AFRL |=  (1 << 0);

    GPIOA->AFRL &= ~(0xF << 4);
    GPIOA->AFRL |=  (1 << 4);


    /* =========================
       MOTOR CONTROL PINS
       =========================

       PB0  -> IN1
       PB1  -> IN2
       PB10 -> IN3
       PB11 -> IN4
    */

    GPIOB->MODER &= ~(3 << 0);
    GPIOB->MODER |=  (1 << 0);

    GPIOB->MODER &= ~(3 << 2);
    GPIOB->MODER |=  (1 << 2);

    GPIOB->MODER &= ~(3 << 20);
    GPIOB->MODER |=  (1 << 20);

    GPIOB->MODER &= ~(3 << 22);
    GPIOB->MODER |=  (1 << 22);


    /* =========================
       TIM2 CLOCK
       ========================= */

    RCC->APB1ENR |= (1 << 0);


    /* =========================
       TIM2 CONFIGURATION
       ========================= */

    TIM2->CR1 = 0;

    /*
     * Timer clock = 16 MHz
     *
     * PSC = 15
     *
     * 16 MHz / (15 + 1)
     * = 1 MHz
     */

    TIM2->PSC = 15;

    /*
     * ARR = 999
     *
     * PWM frequency:
     *
     * 1 MHz / 1000
     * = 1 kHz
     */

    TIM2->ARR = 999;

    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;


    /* =========================
       PWM CHANNEL 1
       ========================= */

    TIM2->CCMR1 &= ~(0xFF);

    /*
     * OC1M = 110
     * PWM mode 1
     */

    TIM2->CCMR1 |= (6 << 4);

    /*
     * OC1PE = 1
     * Preload enable
     */

    TIM2->CCMR1 |= (1 << 3);


    /* =========================
       PWM CHANNEL 2
       ========================= */

    /*
     * OC2M = 110
     */

    TIM2->CCMR1 |= (6 << 12);

    /*
     * OC2PE = 1
     */

    TIM2->CCMR1 |= (1 << 11);


    /* Enable CH1 */
    TIM2->CCER |= (1 << 0);

    /* Enable CH2 */
    TIM2->CCER |= (1 << 4);


    /* Generate update event */
    TIM2->EGR |= (1 << 0);


    /* Start timer */
    TIM2->CR1 |= (1 << 0);


    /* Initial state */
    current_pwm = 0;
    target_pwm = 0;
    motor_direction = MOTOR_STOPPED;
    requested_direction = MOTOR_STOPPED;

    safety_pwm_limit = MOTOR_TARGET_PWM;

    motor_stop();
}


/* =========================
   MOTOR COMMANDS
   ========================= */

void motor_forward(void)
{
    requested_direction = MOTOR_FORWARD;
    target_pwm = MOTOR_TARGET_PWM;
}


void motor_backward(void)
{
    requested_direction = MOTOR_BACKWARD;
    target_pwm = MOTOR_TARGET_PWM;
}


void motor_right(void)
{
    requested_direction = MOTOR_RIGHT;
    target_pwm = MOTOR_TARGET_PWM;
}


void motor_left(void)
{
    requested_direction = MOTOR_LEFT;
    target_pwm = MOTOR_TARGET_PWM;
}


void motor_stop(void)
{
    requested_direction = MOTOR_STOPPED;
    target_pwm = 0;
}


/* =========================
   SAFETY PWM LIMIT
   ========================= */

void motor_set_safety_limit(uint16_t limit)
{
    if (limit > MOTOR_TARGET_PWM)
        limit = MOTOR_TARGET_PWM;

    safety_pwm_limit = limit;

    /* Apply safety limit immediately */
    if (current_pwm > safety_pwm_limit)
    {
        current_pwm = safety_pwm_limit;

        TIM2->CCR1 = current_pwm;
        TIM2->CCR2 = current_pwm;
    }
}


/* =========================
   UPDATE MOTOR SPEED
   ========================= */
void motor_update_speed(void)
{
    uint16_t effective_target;

    /* =========================
       APPLY SAFETY LIMIT
       ========================= */

    effective_target = target_pwm;

    if (effective_target > safety_pwm_limit)
    {
        effective_target = safety_pwm_limit;
    }


    /* =========================
       MOTOR CURRENTLY STOPPED
       ========================= */

    if (motor_direction == MOTOR_STOPPED)
    {
        current_pwm = 0;

        TIM2->CCR1 = 0;
        TIM2->CCR2 = 0;

        /*
         * A new direction is selected
         * only when PWM is already zero.
         */

        if (requested_direction != MOTOR_STOPPED)
        {
            /*
             * Wait for a small amount of time
             * before applying the new direction.
             */

            if (direction_delay < DIRECTION_DELAY)
            {
                direction_delay++;
                return;
            }

            /*
             * PWM is zero and delay is complete.
             * Now change the direction.
             */

            motor_direction = requested_direction;

            set_motor_direction(motor_direction);

            /*
             * New direction starts from zero PWM.
             */

            direction_delay = 0;

            return;
        }

        direction_delay = 0;

        return;
    }


    /* =========================
       DIRECTION CHANGE
       ========================= */

    if (requested_direction != motor_direction)
    {
        /*
         * First reduce PWM smoothly to zero.
         */

        if (current_pwm > 0)
        {
            if (current_pwm > MOTOR_STEP)
            {
                current_pwm -= MOTOR_STEP;
            }
            else
            {
                current_pwm = 0;
            }

            TIM2->CCR1 = current_pwm;
            TIM2->CCR2 = current_pwm;

            /*
             * Make sure the delay starts
             * only after PWM reaches zero.
             */

            direction_delay = 0;

            return;
        }


        /*
         * PWM has reached zero.
         */

        TIM2->CCR1 = 0;
        TIM2->CCR2 = 0;


        /*
         * Keep motor stopped for a short time
         * before changing direction.
         */

        if (direction_delay < DIRECTION_DELAY)
        {
            direction_delay++;
            return;
        }


        /* =========================
           STOP REQUEST
           ========================= */

        if (requested_direction == MOTOR_STOPPED)
        {
            motor_direction = MOTOR_STOPPED;

            set_motor_direction(MOTOR_STOPPED);

            direction_delay = 0;

            return;
        }


        /* =========================
           CHANGE DIRECTION
           ========================= */

        motor_direction = requested_direction;

        set_motor_direction(motor_direction);

        /*
         * Direction changed at zero PWM.
         * Acceleration starts on next update.
         */

        direction_delay = 0;

        return;
    }


    /* =========================
       ACCELERATION
       ========================= */

    if (current_pwm < effective_target)
    {
        current_pwm += MOTOR_STEP;

        if (current_pwm > effective_target)
        {
            current_pwm = effective_target;
        }
    }


    /* =========================
       DECELERATION
       ========================= */

    else if (current_pwm > effective_target)
    {
        if (current_pwm > MOTOR_STEP)
        {
            current_pwm -= MOTOR_STEP;
        }
        else
        {
            current_pwm = 0;
        }
    }


    /* =========================
       STOP CONDITION
       ========================= */

    if ((requested_direction == MOTOR_STOPPED) &&
        (current_pwm == 0))
    {
        motor_direction = MOTOR_STOPPED;

        set_motor_direction(MOTOR_STOPPED);

        TIM2->CCR1 = 0;
        TIM2->CCR2 = 0;

        direction_delay = 0;

        return;
    }


    /* =========================
       APPLY PWM
       ========================= */

    TIM2->CCR1 = current_pwm;
    TIM2->CCR2 = current_pwm;
}
/* =========================
   GET SPEED %
   ========================= */

uint8_t motor_get_speed_percent(void)
{
    return (uint8_t)((current_pwm * 100U) /
                     MOTOR_PWM_MAX);
}


/* =========================
   GET CURRENT DIRECTION
   ========================= */

uint8_t motor_get_direction(void)
{
    return motor_direction;
}
