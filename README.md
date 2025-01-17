# IMECoreKeylogger  

  
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)](#)
[![Language](https://img.shields.io/badge/language-C%2B%2B-blue.svg)](#)    
[English](./README.md) | [中文](./README-zh.md)  
  
IMECoreKeylogger is a research project dedicated to capturing Unicode character inputs (e.g., Chinese, Japanese) that traditional keyloggers often fail to detect. In Windows, especially for non-Latin input, characters may be composed via IME (Input Method Editor) or other frameworks like TSF (Text Services Framework), making them harder to intercept with standard keylogging techniques.

> **Disclaimer**: This project is for **research and educational purposes only**. The author(s) do not assume responsibility for any malicious use of the code or ideas presented here.

---

## Table of Contents

1. [Overview](#overview)  
2. [Windows Input Methods](#windows-input-methods)  
4. [Approaches to Intercepting Input](#approaches-to-intercepting-input)  
5. [Current Implementation](#current-implementation)  

---

## Overview

Traditional keyloggers typically rely on capturing keystrokes through the Win32 message loop, which can be insufficient for capturing Unicode inputs from IMEs or TSF. IMECoreKeylogger aims to explore and demonstrate various techniques to intercept these inputs.  

**Why is it challenging to log Unicode inputs?**  
- When using IME or TSF, keystrokes are not immediately passed to the application as characters.
- Multiple keystrokes may be combined into a single character (e.g., composing Chinese characters via pinyin input).
- TSF abstracts input handling, making it difficult to intercept at the application level.

---

## Windows Input Methods

Windows primarily handles text input through three major layers:

1. **Win32 Message Loop**  
   - Typically uses `GetMessage` and `DispatchMessage` to pass messages to a `WindowProc`.
   - Messages like `WM_CHAR` or `WM_KEYDOWN` are commonly intercepted here.

2. **IME (Input Method Editor)**  
   - Processes keystrokes for complex character composition (e.g., Chinese, Japanese).
   - Key events pass through:
     - `WM_IME_STARTCOMPOSITION`
     - `WM_IME_COMPOSITION`
     - `WM_IME_ENDCOMPOSITION`
   - Finally, `WM_CHAR` sends the composed Unicode character to the application.

3. **TSF (Text Services Framework)**  
   - Integrates multiple text services system-wide.
   - The TSF Manager coordinates text services (TIPs, including IME) and the application’s Text Store.
   - Because TSF bypasses the direct message loop in many cases, conventional keylogging techniques may fail to capture these inputs.


The data flow can be segmented into three layers:

- **Application**: The end-user application.
- **IMM (Input Method Manager)**: Manages IME communication.
- **IME (Input Method Editor)**: Responsible for converting raw keystrokes to composed text.

---

## Approaches to Intercepting Input

IMECoreKeylogger explores several potential methods to capture Unicode inputs:

1. **Injecting into the Foreground Application**  
   - Inject a DLL into the foreground window to hook messages like `WM_CHAR`/`WM_IME_CHAR`.
   - **Pros**: Straightforward to implement.  
   - **Cons**: Continuous injection may be detected by security software or the user.

2. **Driver-Level Interception**  
   - Utilize a driver (potentially hooking into IMM) at ring0 to intercept `WM_CHAR`.
   - **Pros**: More robust and harder to circumvent.  
   - **Cons**: Requires in-depth Windows driver development knowledge.

3. **Replacing IME Files or user32.dll**  
   - Replace or modify system IME libraries (or user32.dll) to intercept Unicode inputs.
   - **Pros**: Centralized interception.  
   - **Cons**: Complex, OS-specific, and very high risk for system instability or detection.

---

## Current Implementation

Our current proof-of-concept focuses on the **first approach**—injecting into the foreground application to capture messages related to Unicode input. The prototype module can be found in the [`ForegroundWindowLogger`](./ForegroundWindowLogger) directory.

---
Readme.md is translated by chat-gpt. Please raise an issue if you have any questions.