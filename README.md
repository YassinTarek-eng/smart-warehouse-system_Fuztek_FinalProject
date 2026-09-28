# Smart Warehouse Inventory & Order Tracking System

## Project Overview
The Smart Warehouse Inventory & Order Tracking System is a C++ desktop final project designed to manage product inventory, supplier relationships, user authentication, and order processing efficiently using Object-Oriented Programming (OOP), design patterns (Singleton), and modular architecture.

---

## Team Members
* **Yassin Tarek Saeed Mohammed Hussein** (Team Leader & Database Specialist)
* **Team Member 1** (User Authentication & Roles)
* **Team Member 2** (Product & Inventory Management)
* **Team Member 3** (Supplier Management)
* **Team Member 4** (Order Processing & Transactions)
* **Team Member 5** (GUI Presentation Layer & Qt Windows)

---

## Prerequisites & Installation Guide

To ensure all parts of the application function properly, All team members should install the required dependencies based on their roles:

### 1. PostgreSQL & C++ Connector (`libpqxx`)
* **Assigned to:** Team Leader (Yassin)
* **Installation Instructions:**
  
  * **Linux (Ubuntu/Debian):**
    ```bash
    sudo apt update
    sudo apt install libpqxx-dev postgresql-client
    ```
  * **Windows (via vcpkg):**
    ```bash
    git clone [https://github.com/microsoft/vcpkg.git](https://github.com/microsoft/vcpkg.git) C:\dev\vcpkg
    C:\dev\vcpkg\bootstrap-vcpkg.bat
    C:\dev\vcpkg\vcpkg install libpqxx
    ```

### 2. Qt Framework (GUI Design)
* **Assigned to:** Team Member 5
* **Installation Instructions:**
  * Download the **Qt Online Installer** from the official Qt website.
  * Install **Qt 6.x** along with **Qt Creator** and your preferred compiler toolchain (MSVC or MinGW for Windows, GCC for Linux).

---
