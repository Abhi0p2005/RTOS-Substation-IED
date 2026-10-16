# Quick Start Guide - IED Protection System

## 🎯 What You Have

A complete **IED Protection Relay System** with:
- ⚡ Real-time protection (ANSI 50 & 51)
- 🖥️ Beautiful Web GUI Dashboard
- 🔧 ESP32 hardware support
- 💻 PC simulator for testing

---

## 🚀 How to Use the GUI (Easiest Way!)

### Step 1: Start the Web Dashboard

```bash
cd /Users/AnirudhKamat7/Abhi/Projects/RTOS_IED_Substation
python3 -m http.server 8080 --directory gui
```

### Step 2: Open in Browser

Open your web browser and go to:
```
http://localhost:8080
```

### Step 3: Test the System

Click the colorful buttons to simulate different fault conditions:

1. **✅ Normal Operation** - 5A current (everything OK)
2. **⚠️ Overload** - 7.5A current (trips after ~5 seconds)
3. **🔥 Heavy Fault** - 15A current (trips after ~1.5 seconds)
4. **💥 Instantaneous** - 40A current (trips immediately in 50ms)

Watch the:
- 📊 **Current meters** change colors
- 🚨 **Trip banner** appear when protection activates
- 📝 **Event log** record everything

---

## 📤 Push to GitHub

Since authentication failed, follow these steps:

### Option 1: Using Personal Access Token (Recommended)

1. **Create a Personal Access Token:**
   - Go to: https://github.com/settings/tokens
   - Click "Generate new token (classic)"
   - Give it a name: "RTOS Project"
   - Check: `repo` (all repo permissions)
   - Click "Generate token"
   - **COPY THE TOKEN** (you won't see it again!)

2. **Push to GitHub:**
```bash
cd /Users/AnirudhKamat7/Abhi/Projects/RTOS_IED_Substation

# Configure git
git config user.name "Abhi0p2005"
git config user.email "your-email@example.com"

# Push using token
git push -u origin main
# When prompted for password, paste your TOKEN (not your password!)
```

### Option 2: Using SSH (Alternative)

1. **Check if you have SSH key:**
```bash
ls ~/.ssh/id_rsa.pub
```

2. **If no key exists, create one:**
```bash
ssh-keygen -t rsa -b 4096 -C "your-email@example.com"
# Press Enter for all prompts
```

3. **Add SSH key to GitHub:**
```bash
cat ~/.ssh/id_rsa.pub
# Copy the output
```
   - Go to: https://github.com/settings/keys
   - Click "New SSH key"
   - Paste your key
   - Click "Add SSH key"

4. **Change remote URL and push:**
```bash
cd /Users/AnirudhKamat7/Abhi/Projects/RTOS_IED_Substation
git remote set-url origin git@github.com:Abhi0p2005/RTOS-Substation-IED.git
git push -u origin main
```

---

## 🏗️ Build & Run (Command Line)

### For Native PC Simulator:

```bash
cd /Users/AnirudhKamat7/Abhi/Projects/RTOS_IED_Substation

# Build
pio run -e native

# Run
.pio/build/native/program
```

### For ESP32 Hardware:

```bash
# Build for ESP32
pio run -e esp32

# Upload to ESP32
pio run -e esp32 -t upload

# Monitor serial output
pio device monitor -e esp32
```

---

## 📁 Project Structure

```
RTOS_IED_Substation/
├── gui/                    # 🌐 Web dashboard (START HERE!)
│   └── index.html
├── src/                    # 💻 Source code
│   ├── main.c             # Main program
│   ├── protection/        # ANSI 50/51 logic
│   ├── hal_sim.c         # PC simulator
│   └── hal_esp32.c       # ESP32 hardware
├── include/               # 📋 Header files
├── README.md             # 📖 Full documentation
└── platformio.ini        # ⚙️ Build configuration
```

---

## 🎮 GUI Dashboard Features

### Status Indicators
- 🟢 Green LED = Normal operation
- 🔴 Red LED = Trip/Fault condition
- ⚪ Gray LED = Inactive

### Current Meters
- Green zone: Normal (< 7.5A)
- Yellow zone: Overload (7.5A - 40A)
- Red zone: Fault (> 40A)

### Protection Settings Display
- **ANSI 50**: Instantaneous OC (40A pickup, 50ms delay)
- **ANSI 51**: Time OC (7.5A pickup, TMS=0.1, IEC curve)

### Event Log
- Real-time events with timestamps
- Fault injection records
- Trip events with details

---

## 🆘 Troubleshooting

### GUI won't open?
```bash
# Kill any existing server
pkill -f "http.server"

# Start fresh
cd /Users/AnirudhKamat7/Abhi/Projects/RTOS_IED_Substation
python3 -m http.server 8080 --directory gui

# Open browser manually to: http://localhost:8080
```

### Build fails?
```bash
# Clean and rebuild
pio run -e native -t clean
pio run -e native
```

### Git push fails?
- Use Personal Access Token (see above)
- Make sure repository exists on GitHub
- Check your internet connection

---

## 📚 What Each Protection Does

### ANSI 50 - Instantaneous Overcurrent
- **Purpose**: Stop BIG faults FAST (like short circuits)
- **Trigger**: Current ≥ 40A (8× rated)
- **Speed**: Trips in 50 milliseconds
- **Example**: Press "💥 Instantaneous" button

### ANSI 51 - Time Overcurrent (IDMT)
- **Purpose**: Stop overloads with coordination
- **Trigger**: Current ≥ 7.5A (1.5× rated)
- **Speed**: Slower for smaller faults (5-10 seconds for 1.5×)
- **Example**: Press "⚠️ Overload" button

---

## 🎓 Understanding the System

Think of it like a smart circuit breaker:
1. **Monitors** 3-phase electrical current constantly
2. **Calculates** RMS values 50 times per second
3. **Detects** when current is too high
4. **Trips** the relay to protect equipment
5. **Logs** everything for analysis

---

## ✅ Next Steps

1. ✨ **Try the GUI** - Most fun and easiest!
2. 📤 **Push to GitHub** - Save your work
3. 🔌 **Test on ESP32** - Real hardware (optional)
4. 📖 **Read README.md** - Learn the details

---

## 📞 Need Help?

Check these files:
- `README.md` - Comprehensive documentation
- `docs/ARCHITECTURE.md` - Technical details
- `MASTER_PROMPT.md` - Original specifications

**Enjoy your IED Protection System! 🎉**
