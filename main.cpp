#include <iostream>
#include <vector>
#include <string>
#include <ctime>

// PostgreSQL C++ Client Library Header
// #include <pqxx/pqxx>

// Note: When you integrate Qt later, your GUI file will include Qt headers like:
// #include <QApplication>
// #include <QMainWindow>
// #include <QTableWidget>

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
// TEAM MEMBER 5: GUI Presentation Layer & Desktop Windows (Qt Framework)
// ============================================================================
class GUIController {
private:
    bool isWindowOpen;

public:
    GUIController() {
        isWindowOpen = false;
    }

    void openMainWindow() {
        // TODO [Member 5]: Initialize Qt window layout and buttons
        cout << "[GUI] Opening Smart Warehouse Desktop Window via Qt..." << endl;
        isWindowOpen = true;
    }

    void displayInventoryTable(const vector<Product>& products) {
        // TODO [Member 5]: Populate QTableWidget widgets on screen with product data
        cout << "[GUI] Rendering inventory table on screen..." << endl;
        for (const auto& p : products) {
            cout << "   -> Table Row: " << p.getName() << " | Stock: " << p.getStock() << endl;
        }
    }

    void showAlertPopup(string message) {
        // TODO [Member 5]: Trigger QMessageBox popup warning dialog on desktop UI
        cout << "[GUI Alert Popup]: " << message << endl;
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
    return 0;
}