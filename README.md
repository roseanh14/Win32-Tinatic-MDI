# Win32-Titanic-MDI

A Win32 API semester project demonstrating **MDI windows**, **Clipboard data transfer with a custom registered format**, **anonymous/named pipes**, and **DDE communication**, using the **Titanic passengers** CSV dataset.

## Project Overview

This project contains two native Windows desktop applications written with the **Win32 API** only.

The goal is to demonstrate several classic Windows interprocess and intra-application communication mechanisms without using MFC, WinUI, or .NET.

### Application A
Application A is the main MDI application. After startup, it automatically creates two MDI child windows:

- **OA1**
- **OA2**

#### OA1 features
OA1 provides commands for:

- displaying loaded Titanic dataset records
- sending the data to **OA2**
- sending the data to the **Clipboard**
- exporting the data to **Application B** through a **Pipe**
- sending the data from **OA1** to **OA2** using **DDE**

#### OA2 features
OA2 is a receiver/viewer window. It can:

- accept data sent internally from OA1
- accept data from the Clipboard using a **custom registered clipboard format**
- display the received table data as text
- receive data through **DDE**

### Application B
Application B is launched as a child process from Application A. It receives the same Titanic dataset through a **Pipe** and displays the received content in its own dialog/window.

---

## Assignment Requirements Covered

This project was implemented according to the following requirements:

- Win32 API only
- MDI parent window with two child windows (**OA1**, **OA2**)
- OA1 menu with commands for:
  - data listing
  - sending data to OA2
  - sending data to Clipboard
  - reading/displaying clipboard data in OA2
- custom registered Clipboard format for **tabular data**
- Titanic dataset used as the project data source
- creation of child process (**Application B**)
- data transfer to Application B via **Pipe**
- data transfer from OA1 to OA2 using **DDE**
- implementation of a custom **DDE server**

---

## Dataset

The project uses the **Titanic passengers** CSV dataset.

The dataset is loaded from the CSV source and then converted into an internal tabular representation used for:

- direct display in OA1
- transfer to OA2
- clipboard export/import
- pipe communication with Application B
- DDE communication

Typical fields include:

- PassengerId
- Survived
- Pclass
- Name
- Sex
- Age
- SibSp
- Parch
- Ticket
- Fare
- Cabin
- Embarked

---

## Technologies Used

- **C++**
- **Win32 API**
- **MDI (Multiple Document Interface)**
- **Clipboard API**
- **Custom registered clipboard format**
- **Pipe communication**
- **DDE (Dynamic Data Exchange)**
- **CMake**
- **Visual Studio / MSVC**

---

