# IMECoreKeylogger

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)](#)
[![Language](https://img.shields.io/badge/language-C%2B%2B-blue.svg)](#)    
[English](./README.md) | [中文](./README-zh.md)  

IMECoreKeylogger 是一個研究項目，旨在研究如何在使用者輸入 Unicode 字元（如中文、日文）時，進行攔截並記錄輸入行為。對於非拉丁字元的輸入而言，Windows 系統中常採用 IME（Input Method Editor）或 TSF（Text Services Framework）等機制，傳統 Keylogger 往往無法有效捕捉這些輸入。

> **免責聲明**：本專案僅用於**研究與教育用途**。作者對於任何惡意使用該程式碼或技術概念所造成的影響概不負責。

---

## 目錄
1. [概述](#概述)  
2. [Windows 輸入方式](#windows-輸入方式)   
3. [攔截使用者輸入的思路](#攔截使用者輸入的思路)  
4. [目前實作方式](#目前實作方式)  


---

## 概述
傳統的 keylogger 通常依賴於 Win32 訊息迴圈來攔截按鍵，無法直接取得 IME 或 TSF 所產生的複合字元。本專案主要探討與展示如何在此情境下攔截輸入。

**為什麼攔截 Unicode 輸入這麼困難？**
- 使用 IME 或 TSF 時，應用程式收到的並非即時字元，而是多次鍵盤輸入後的合成結果。
- 可能需要多次按鍵才能輸入一個完整的字元（如中文拼音輸入）。
- TSF 對輸入流程進行抽象，導致單純在應用層面難以攔截。

---

## Windows 輸入方式

1. **Win32 訊息迴圈**  
   - 通常使用 `GetMessage` 與 `DispatchMessage` 來將訊息傳遞給 `WindowProc`。
   - 常見的訊息包含 `WM_CHAR`、`WM_KEYDOWN` 等。

2. **IME（Input Method Editor）**  
   - 負責複雜字元的拼音與輸入，如中文、日文。
   - 包含以下訊息：
     - `WM_IME_STARTCOMPOSITION`
     - `WM_IME_COMPOSITION`
     - `WM_IME_ENDCOMPOSITION`
   - 最終透過 `WM_CHAR` 將所組合的字元傳給應用程式。

3. **TSF（Text Services Framework）**  
   - 系統層的文字服務整合機制。
   - TSF Manager 負責協調文字服務（TIP，包括 IME）與應用程式的 Text Store。
   - 由於 TSF 可能跳過直接的 Win32 訊息迴圈，所以一般 keylogger 難以攔截。
  
整個輸入流程大致可分為以下三層：
- **Application（應用程式）**
- **IMM（Input Method Manager）**
- **IME（Input Method Editor）**

---

## 攔截使用者輸入的思路

1. **注入前台應用程式**  
   - 注入 DLL 到前台視窗，以攔截 `WM_CHAR` / `WM_IME_CHAR` 等訊息。
   - **優點**：實作相對簡單。  
   - **缺點**：需反覆注入，容易被防毒或使用者發現。

2. **驅動層攔截**  
   - 利用 Driver（可能是 IMM）在 ring0 層攔截 `WM_CHAR`。
   - **優點**：更底層，更難被繞過。  
   - **缺點**：需要深入的 Windows 驅動開發知識。

3. **替換 IME 或 user32.dll**  
   - 透過替換或修改系統 IME 檔案（或 user32.dll）達成攔截。
   - **優點**：攔截位置集中。
   - **缺點**：涉及系統檔案修改，風險與複雜度都很高。

---

## 目前實作方式

本專案目前聚焦於第一種方法：**注入前台視窗**，攔截由 IME 輸入的字元訊息。可在 [`ForegroundWindowLogger`](./ForegroundWindowLogger) 目錄中找到目前的原型實作程式。

---
Readme.md由chat-gpt翻譯，若有問題請提出issue