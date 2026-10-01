#include <iostream>
#include <vector>
#include <string>
#include <ctime>

// PostgreSQL C++ Client Library Header
#include <pqxx/pqxx>

// FLTK GUI headers (replaces the old Qt headers)
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>

using namespace std;

// ============================================================================
// TEAM LEADER: YASSIN
// RESPONSIBILITY: PostgreSQL Connection & Singleton Pattern (using libpqxx)
// ============================================================================
class DatabaseManager {
private:
    string connectionString;
    bool isConnected;

    // Private constructor prevents creating multiple instances (Singleton)
    DatabaseManager() {
        // Connection string for PostgreSQL database
        connectionString = "dbname=warehouse_db user=postgres password=secret host=127.0.0.1 port=5432";
        isConnected = false;
    }

public:
    // Global access point to get the single database instance
    static DatabaseManager& getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    bool connect() {
        try {
            // Using libpqxx connection object
            pqxx::connection conn(connectionString);
            if (conn.is_open()) {
                cout << "[Database] Successfully connected to PostgreSQL: " << conn.dbname() << endl;
                isConnected = true;
            } else {
                isConnected = false;
            }
        } catch (const exception& e) {
            cerr << "[Database Error]: " << e.what() << endl;
            isConnected = false;
        }
        return isConnected;
    }

    void executeQuery(const string& sqlQuery) {
        try {
            pqxx::connection conn(connectionString);
            pqxx::work txn(conn);
            txn.exec(sqlQuery);
            txn.commit();
            cout << "[Database] Successfully executed SQL: " << sqlQuery << endl;
        } catch (const exception& e) {
            cerr << "[Query Error]: " << e.what() << endl;
        }
    }

    // Delete copy constructor and assignment operator for safety
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;
};


// ============================================================================
// TEAM MEMBER 1: User Authentication & Roles
// ============================================================================
class User {
private:
    int userId;
    string username;
    string passwordHash;
    string role; // "Clerk" or "Manager"

public:
    User(int id, string uname, string pwd, string r) {
        userId = id;
        username = uname;
        passwordHash = pwd;
        role = r;
    }

    string getUsername() const {
        // TODO [Member 1]: Return username safely
        return username;
    }

    string getRole() const {
        // TODO [Member 1]: Return user role
        return role;
    }

    bool authenticate(string inputPassword) const {
        // TODO [Member 1]: Verify password against passwordHash
        if (passwordHash == inputPassword) {
            return true;
        }
        return false;
    }

    void displayUser() const {
        // TODO [Member 1]: Print user details cleanly
        cout << "User: " << username << " [Role: " << role << "]" << endl;
    }
};


// ============================================================================
// TEAM MEMBER 2: Product & Inventory Management
// ============================================================================
class Category {
private:
    int categoryId;
    string categoryName;

public:
    Category(int id, string name) {
        categoryId = id;
        categoryName = name;
    }

    string getCategoryName() const {
        // TODO [Member 2]: Return category name
        return categoryName;
    }
};

class Product {
private:
    int productId;
    string productName;
    double unitPrice;
    int stockQuantity;
    int categoryId;

public:
    Product() {
        productId = 0;
        productName = "";
        unitPrice = 0.0;
        stockQuantity = 0;
        categoryId = 0;
    }

    Product(int id, string name, double price, int stock, int catId = 1) {
        productId = id;
        productName = name;
        unitPrice = price;
        stockQuantity = stock;
        categoryId = catId;
    }

    int getId() const {
        // TODO [Member 2]: Return product ID
        return productId;
    }

    string getName() const {
        // TODO [Member 2]: Return product name
        return productName;
    }

    double getPrice() const {
        // TODO [Member 2]: Return product unit price
        return unitPrice;
    }

    int getStock() const {
        // TODO [Member 2]: Return available stock quantity
        return stockQuantity;
    }

    void updateStock(int amount) {
        // TODO [Member 2]: Add or subtract stock quantity with validation
        stockQuantity += amount;
        if (stockQuantity < 0) {
            stockQuantity = 0;
        }
    }

    void displayProduct() const {
        // TODO [Member 2]: Format and print product info
        cout << "Product ID: " << productId 
             << " | Name: " << productName 
             << " | Price: $" << unitPrice 
             << " | Stock: " << stockQuantity << endl;
    }
};


// ============================================================================
// TEAM MEMBER 3: Supplier Management
// ============================================================================
class Supplier {
private:
    int supplierId;
    string supplierName;
    string contactEmail;
    vector<int> suppliedProductIds;

public:
    Supplier(int id, string name, string email) {
        supplierId = id;
        supplierName = name;
        contactEmail = email;
    }

    int getId() const {
        // TODO [Member 3]: Return supplier ID
        return supplierId;
    }

    string getName() const {
        // TODO [Member 3]: Return supplier name
        return supplierName;
    }

    void addSuppliedProduct(int productId) {
        // TODO [Member 3]: Link product ID to this supplier
        suppliedProductIds.push_back(productId);
    }

    void displaySupplier() const {
        // TODO [Member 3]: Print supplier details and vendor contact info
        cout << "Supplier: " << supplierName << " (Contact: " << contactEmail << ")" << endl;
    }
};


// ============================================================================
// TEAM MEMBER 4: Order Processing & Transactions
// ============================================================================
class Order {
private:
    int orderId;
    vector<pair<Product, int>> orderedItems;
    double totalAmount;
    string status; // "Pending", "Completed", "Cancelled"

public:
    Order(int id) {
        orderId = id;
        totalAmount = 0.0;
        status = "Pending";
    }

    void addProduct(const Product& product, int quantity) {
        // TODO [Member 4]: Add product to order and update total cost
        orderedItems.push_back({product, quantity});
        totalAmount += product.getPrice() * quantity;
    }

    void setStatus(string newStatus) {
        // TODO [Member 4]: Update order status
        status = newStatus;
    }

    void displayOrder() const {
        // TODO [Member 4]: Print complete order receipt and status details
        cout << "========================================" << endl;
        cout << "ORDER RECEIPT #" << orderId << " [Status: " << status << "]" << endl;
        cout << "----------------------------------------" << endl;
        
        for (const auto& item : orderedItems) {
            cout << "  - " << item.first.getName() 
                 << " x " << item.second 
                 << " @ $" << item.first.getPrice() 
                 << " = $" << (item.first.getPrice() * item.second) << endl;
        }
        
        cout << "----------------------------------------" << endl;
        cout << "Total Cost: $" << totalAmount << endl;
        cout << "========================================" << endl;
    }
};


// ============================================================================
// TEAM MEMBER 5: GUI Presentation Layer & Desktop Windows (FLTK Framework)
// ============================================================================
class GUIController {
private:
    bool isWindowOpen;
    Fl_Window* window;
    Fl_Hold_Browser* inventoryTable;

public:
    GUIController() {
        isWindowOpen = false;
        window = nullptr;
        inventoryTable = nullptr;
    }

    void openMainWindow() {
        // TODO [Member 5]: Add more buttons, tabs and layout to the FLTK window
        cout << "[GUI] Opening Smart Warehouse Desktop Window via FLTK..." << endl;
        window = new Fl_Window(700, 450, "Smart Warehouse System");
        inventoryTable = new Fl_Hold_Browser(10, 10, 680, 380);
        Fl_Button* closeBtn = new Fl_Button(590, 400, 100, 40, "Close");
        closeBtn->callback([](Fl_Widget*, void* w) { ((Fl_Window*)w)->hide(); }, window);
        window->end();
        window->show();
        isWindowOpen = true;
    }

    void displayInventoryTable(const vector<Product>& products) {
        // TODO [Member 5]: Add columns (use column_widths) and refresh logic
        cout << "[GUI] Rendering inventory table on screen..." << endl;
        inventoryTable->clear();
        for (const auto& p : products) {
            string row = p.getName() + " | Stock: " + to_string(p.getStock());
            inventoryTable->add(row.c_str());
            cout << "   -> Table Row: " << row << endl;
        }
    }

    void showAlertPopup(string message) {
        // TODO [Member 5]: Customize the warning dialog if needed
        cout << "[GUI Alert Popup]: " << message << endl;
        fl_alert("%s", message.c_str());
    }
};


// ============================================================================
// MAIN APPLICATION ENTRY POINT (Integration & Testing)
// ============================================================================
int main() {
    cout << "=== Smart Warehouse Inventory & Order Tracking System ===" << endl << endl;

    // 1. Initialize Database Connection (Leader Yassin)
    DatabaseManager& db = DatabaseManager::getInstance();
    db.connect();

    // 2. Test User Authentication (Member 1)
    User clerk(1, "warehouse_clerk", "pass123", "Clerk");
    clerk.displayUser();

    // 3. Test Inventory & Products (Member 2)
    vector<Product> productList;
    productList.push_back(Product(101, "Steel Bracket", 12.50, 200, 1));
    productList.push_back(Product(102, "Conveyor Belt Motor", 450.00, 5, 2));

    cout << "\nCurrent Inventory:" << endl;
    for (const auto& prod : productList) {
        prod.displayProduct();
    }

    // 4. Test Supplier Management (Member 3)
    Supplier supplier(501, "Industrial Parts Ltd", "sales@indparts.com");
    supplier.addSuppliedProduct(101);
    supplier.displaySupplier();

    // 5. Test Order Processing (Member 4)
    Order myOrder(9001);
    myOrder.addProduct(productList[0], 10);
    myOrder.setStatus("Completed");
    myOrder.displayOrder();

    // 6. Test GUI Controller (Member 5)
    GUIController gui;
    gui.openMainWindow();
    gui.displayInventoryTable(productList);
    gui.showAlertPopup("Low Stock Alert: Conveyor Belt Motor has only 5 units left!");

    cout << "\nProgram executed successfully." << endl;

    // Keep the window open until the user closes it
    return Fl::run();
}