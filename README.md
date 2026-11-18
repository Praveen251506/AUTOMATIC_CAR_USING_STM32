# 🚗 Bluetooth + Tilt Controlled Smart Car

> What if your car listened to your phone AND knew when it was tilting? 👀

A little embedded-systems experiment where Bluetooth meets motion sensing. Control the car wirelessly using your phone, while an accelerometer keeps an eye on its tilt and automatically adjusts motor speed.

Built with **STM32F407, Embedded C, Bluetooth and a bunch of electronics magic.**

## 🎮 What can this little machine do?

- 📱 **Bluetooth Control** – Drive forward, backward, left and right using your phone.
- 📐 **Tilt Detection** – Detects vehicle tilt using an LIS302DL accelerometer.
- ⚡ **Smart Speed Control** – Automatically reduces speed based on tilt conditions.
- 💡 **Automatic Headlights** – Uses an LDR to detect surrounding light.
- 📺 **Live Status** – Displays vehicle information on a 20x4 LCD.
- 🔧 **Bare-Metal Programming** – No STM32 HAL. Just direct register-level programming!

## 🧠 The Hardware Behind the Madness

| Component | Job |
|---|---|
| STM32F407 | The brain 🧠 |
| HC-06 | Bluetooth communication 📡 |
| LIS302DL | Tilt detection 📐 |
| L298N | Motor control ⚙️ |
| DC Motors | Makes things move 🚗 |
| LDR | Detects light 💡 |
| 20x4 LCD | Displays what's happening 📺 |

## ⚙️ How does it work?

Pretty simple:

```text
Phone 📱
   |
 Bluetooth (HC-06)
   |
   v
 STM32F407 🧠
   |
   +---- Motor Control
   |
   +---- Tilt Detection
   |
   +---- Light Detection
   |
   +---- LCD Display
```

The phone sends movement commands through Bluetooth. STM32 processes them and controls the motors.

Meanwhile, the accelerometer continuously monitors the car's orientation.

### 📐 The Tilt Logic

The car has three speed conditions:

| Condition | PWM | Action |
|---|---:|---|
| 🟢 Normal | 300 | Full configured speed |
| 🟡 Warning | 150 | Slow down |
| 🔴 Danger | 0 | Stop motors |

Simple logic. Smarter movement.

*PWM values depend on the configured timer period.*

## 🛠️ Tech Stack

- **Language:** Embedded C
- **Microcontroller:** STM32F407
- **Communication:** USART2, SPI1, I2C
- **Motor Control:** TIM2 PWM + GPIO
- **Sensor Interface:** LIS302DL + ADC
- **Development:** STM32CubeIDE / ARM GCC

One thing worth mentioning: we configured the peripherals using direct register access instead of relying on STM32 HAL libraries. So yes, we got our hands dirty with the registers. 😎

## 🚀 Let's Build It!

### 1. Clone the repository

```bash
git clone https://github.com/YOUR_USERNAME/YOUR_REPO.git
cd YOUR_REPO
```

### 2. Get your hardware ready

Grab the STM32F407, HC-06, LIS302DL, L298N, motors, LDR and LCD.

### 3. Open the project

Import the project into STM32CubeIDE and check the target MCU and peripheral configuration.

### 4. Flash it

Connect your ST-LINK debugger, build the firmware and flash it to the STM32.

### 5. Pair and drive!

Pair your phone with HC-06, open a Bluetooth terminal app, and send the movement commands supported by the firmware.

**And boom! Your car is ready to roll.** 🚗

> ⚠️ Before powering up, double-check your wiring, supply voltages, motor driver connections and STM32 pin configuration.

## 📂 Repository Structure

```text
📁 Bluetooth-Tilt-Car
 ┣ 📁 Core
 ┃ ┣ 📁 Inc
 ┃ ┗ 📁 Src
 ┣ 📁 Drivers
 ┣ 📁 Modules
 ┣ 📁 Hardware
 ┣ 📁 Images
 ┣ 📜 README.md
 ┗ 📜 LICENSE
```

*The exact structure may vary depending on the project source organization.*

## 🎯 Why this project?

Because embedded systems are more fun when you stop blinking LEDs and start making things move!

This project helped explore:
- Bare-metal STM32 programming.
- UART Bluetooth communication.
- SPI sensor interfacing.
- I2C communication.
- ADC and PWM.
- Real-world motor control.
- Hardware-software integration.

## 🔮 What's Next?

Some ideas for the next version:

- Ultrasonic obstacle detection.
- Autonomous driving.
- Better tilt estimation.
- Smartphone-based dashboard.
- Battery monitoring.

## 🤝 Want to Experiment?

Fork it, build it, break it, improve it!

If you find something interesting or have an improvement, feel free to open an issue or submit a pull request.

---

**Made with ⚡, Embedded C and a little bit of curiosity.**

*Happy Building! 🚀*
