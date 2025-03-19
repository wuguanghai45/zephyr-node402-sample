#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
LOG_MODULE_REGISTER(main);

#include <string>
#include <utility>
#include <vector>

#include <hc/canopen_stack.hpp>
#include <hc/node402.hpp>
#include "motor_settings.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


enum CanbusDef : uint8_t
{
    CAN_1   = 1,
    CAN_NUM = 1,
};

void node402_event_emitter(Node402Event e) {
    LOG_INF("Got motor event: %d", e.type);
}

Node402* motor;  // Use a pointer for the motor
int target = 100000;

// Function to control motor direction
void control_motor_direction(bool forward) {
    if (forward) {
        motor->set_target_position(target);
    } else {
        motor->set_target_position(-target);
    }
    motor->start_profile_position_mode_1(true);
    motor->start_profile_position_mode_2(true, true);
}

// Shell command handler for motor control
static int cmd_motor_control(const struct shell *shell, size_t argc, char **argv) {
    if (argc != 2) {
        shell_print(shell, "Usage: motor <forward|reverse>");
        return -EINVAL;
    }

    if (strcmp(argv[1], "forward") == 0) {
        control_motor_direction(true);
        shell_print(shell, "Motor set to forward");
    } else if (strcmp(argv[1], "reverse") == 0) {
        control_motor_direction(false);
        shell_print(shell, "Motor set to reverse");
    } else {
        shell_print(shell, "Invalid argument. Use 'forward' or 'reverse'.");
        return -EINVAL;
    }

    return 0;
}

// Register shell command
SHELL_CMD_REGISTER(motor, NULL, "Control motor direction", cmd_motor_control);


int main(void)
{
    int ret;

    CANopenStack canopen_stack(DEVICE_DT_GET(DT_ALIAS(can1)), 1000, CAN_1);

    canopen_stack.init();

    // Initialize the motor using a pointer
    motor = new Node402{&canopen_stack, 4, &node402_1_bin[0], node402_event_emitter};

    canopen_stack.register_node(*motor);
    canopen_stack.start();
    canopen_stack.reset_all_node();
    canopen_stack.operational();

    motor->init();
    motor->enable();
    
    motor->set_profile_position_mode();
    motor->set_target_position(target);
    motor->set_profile_velocity(200000);
    motor->set_profile_acceleration(50000);
    motor->set_profile_deceleration(50000);
    motor->start_profile_position_mode_1(true);
    motor->start_profile_position_mode_2(true, true);

    uint32_t data;
    size_t read_size;
    ret = canopen_stack.get_sdo(4, 0x1000, 0, (uint8_t *)&data, 4, &read_size);
    if (!ret)
    {
        LOG_INF("Device type: 0x%" PRIx32, data);
    }
    else
    {
        LOG_ERR("SDO Error: %" PRIi32, ret);
    }

    if (!device_is_ready(led.port))
    {
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        return ret;
    }
    int t = 0;
    while (1)
    {
        k_msleep(SLEEP_TIME_MS);
    }

    // Clean up
    delete motor;

    return 0;
}
