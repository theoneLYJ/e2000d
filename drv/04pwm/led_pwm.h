#ifndef __LED_PWM_H__
#define __LED_PWM_H__

#define MAP_SIZE 4094UL
#define MAP_MASK (MAP_SIZE - 1)

#define PWM_ENABLE_REG										"0x2807e020"
#define PWM_ENABLE_VAL										"0xff"
#define GPIO_PAD_MODE_PWM0_CHANNEL0_REG 	"0x32b30050"
#define GPIO_PAD_MODE_PWM2_CHANNEL0_REG 	"0x32b30060"
#define GPIO_PAD_MODE_PWM0_CHANNEL0_VAL 	"0x241"
#define GPIO_PAD_MODE_PWM2_CHANNEL0_VAL 	"0x241"
#define PWM0_CONTROLLER										"0"
#define PWM2_CONTROLLER										"2"
#define PWM0_CHANNEL											"0"
#define PWM2_CHANNEL											"2"
#define PWM_DUTY1													"0"
#define PWM_DUTY2													"100000"

#define	WRITE_TYPE_W											"W"
#define	WRITE_TYPE_H											"H"
#define	WRITE_TYPE_B											"B"

enum PWMX {
	PWM0,
	PWM2,
	PWM0_2,
	PWMMAX
};

int pwm_init(void);
int pwm_open(int pwmx, int step);

#endif // !__LED_PWM_H__
