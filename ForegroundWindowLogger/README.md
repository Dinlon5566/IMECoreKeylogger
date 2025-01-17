# ForegroundWindowLogger

POC Video:  
![IME_Core_Keylogger_Youtube](https://youtu.be/-JPia2ge85I?si=yp7CvjYP7zK6T4tM)

In this program, I hooked **GetMessage** to obtain the Unicode characters (and ASCII) that IMM uses this API to send to the application. This is how these keyloggers work.

---

## Usage

1. **Prepare the MinHook Library**  
   First, compile the [MinHook](https://github.com/TsudaKageyu/minhook) library to obtain the necessary `.lib` files, then place them in the project directory.

2. **Build the Project**  
   - Open the solution (`.sln`) file in Visual Studio or another supported IDE.
   - Compile the project.

3. **Run the Sample**  
   - Open a command prompt (cmd) with administrative privileges.
   - Execute the following command to load and run the DLL:
     ```bash
     rundll32 IMECoreLogger.dll,IMELoggerEntry
     ```
The record will be stored in IMEKeyInputlogger.txt.