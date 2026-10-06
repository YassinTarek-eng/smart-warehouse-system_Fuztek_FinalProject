#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

// PostgreSQL C++ Client Library Header (must come BEFORE windows.h)
#include <pqxx/pqxx>

#undef UNICODE
#undef _UNICODE
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#include <cstdio>
#include <cstdlib>

using namespace std;

// ============================================================================
// TEAM MEMBER 2: Product & Inventory Management (Defined first so DatabaseManager can use it)
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
        return productId;
    }

    string getName() const {
        return productName;
    }

    double getPrice() const {
        return unitPrice;
    }

    int getStock() const {
        return stockQuantity;
    }

    void updateStock(int amount) {
        stockQuantity += amount;
        if (stockQuantity < 0) {
            stockQuantity = 0;
        }
    }

    void displayProduct() const {
        cout << "Product ID: " << productId 
             << " | Name: " << productName 
             << " | Price: $" << unitPrice 
             << " | Stock: " << stockQuantity << endl;
    }
};


// ============================================================================
// TEAM LEADER: YASSIN
// RESPONSIBILITY: PostgreSQL Connection, Singleton Pattern & Table Setup (using libpqxx)
// ============================================================================
class DatabaseManager {
private:
    pqxx::connection* conn;
    bool isConnectedStatus;

    // Private constructor prevents creating multiple instances (Singleton)
    DatabaseManager() {
        try {
            string conn_str = "dbname=smart_warehouse user=postgres password=postgres host=127.0.0.1 port=5432";
            conn = new pqxx::connection(conn_str);
            if (conn->is_open()) {
                cout << "[Database] Successfully connected to PostgreSQL: " << conn->dbname() << endl;
                isConnectedStatus = true;
            } else {
                isConnectedStatus = false;
            }
        } catch (const exception& e) {
            cerr << "[Database Error]: " << e.what() << endl;
            conn = nullptr;
            isConnectedStatus = false;
        }
    }

    ~DatabaseManager() {
        if (conn) {
            delete conn;
            conn = nullptr;
        }
    }

public:
    // Delete copy constructor and assignment operator for safety (Singleton pattern)
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Global access point to get the single database instance
    static DatabaseManager& getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    bool connect() {
        return isConnectedStatus && conn != nullptr && conn->is_open();
    }

    bool executeQuery(const string& sqlQuery) {
        try {
            if (!conn || !conn->is_open()) return false;
            pqxx::work txn(*conn);
            txn.exec(sqlQuery);
            txn.commit();
            cout << "[Database] Successfully executed SQL: " << sqlQuery << endl;
            return true;
        } catch (const exception& e) {
            cerr << "[Query Error]: " << e.what() << endl;
            return false;
        }
    }

    bool deleteProductFromDB(int productId) {
        try {
            if (!conn || !conn->is_open()) return false;
            pqxx::work txn(*conn);
            
            // 1. Delete the specific product
            string sql = "DELETE FROM products WHERE id = " + to_string(productId) + ";";
            txn.exec(sql);
            
            // 2. Check if the table is now completely empty
            pqxx::result checkRes = txn.exec("SELECT COUNT(*) FROM products;");
            if (!checkRes.empty() && checkRes[0][0].as<int>() == 0) {
                // If empty, reset the auto-incrementing ID sequence back to 1
                txn.exec("ALTER SEQUENCE products_id_seq RESTART WITH 1;");
                cout << "[Database] Table is empty. ID sequence reset to 1." << endl;
            }
            
            txn.commit();
            cout << "[Database] Product deleted from PostgreSQL: ID " << productId << endl;
            return true;
        } catch (const exception& e) {
            cerr << "[Delete Error]: " << e.what() << endl;
            return false;
        }
    }

    void initializeTables() {
        string usersTable = "CREATE TABLE IF NOT EXISTS users ("
                             "id SERIAL PRIMARY KEY, "
                             "username VARCHAR(64) UNIQUE NOT NULL, "
                             "password VARCHAR(64) NOT NULL, "
                             "role VARCHAR(32) NOT NULL);";

        string productsTable = "CREATE TABLE IF NOT EXISTS products ("
                                "id SERIAL PRIMARY KEY, "
                                "name VARCHAR(64) NOT NULL, "
                                "quantity INT NOT NULL, "
                                "price NUMERIC(10,2) NOT NULL, "
                                "category_id INT, "
                                "supplier_id INT);";

        executeQuery(usersTable);
        executeQuery(productsTable);
        cout << "[Database] Warehouse database tables initialized successfully." << endl;
    }

    vector<Product> loadProductsFromDB() {
        vector<Product> dbProducts;
        try {
            if (!conn || !conn->is_open()) return dbProducts;
            
            pqxx::work txn(*conn);
            // Added ORDER BY id ASC to keep the list neatly sorted
            pqxx::result res = txn.exec("SELECT id, name, price, quantity FROM products ORDER BY id ASC;");
            
            for (auto row : res) {
                int id = row["id"].as<int>();
                string name = row["name"].as<string>();
                double price = row["price"].as<double>();
                int quantity = row["quantity"].as<int>();
                
                dbProducts.push_back(Product(id, name, price, quantity));
            }
            cout << "[Database] Successfully loaded " << dbProducts.size() << " products from PostgreSQL." << endl;
        } catch (const exception& e) {
            cerr << "[Load Error]: " << e.what() << endl;
        }
        return dbProducts;
    }
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
        return username;
    }

    string getRole() const {
        return role;
    }

    bool authenticate(string inputPassword) const {
        if (passwordHash == inputPassword) {
            return true;
        }
        return false;
    }

    void displayUser() const {
        cout << "User: " << username << " [Role: " << role << "]" << endl;
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
        return supplierId;
    }
 
    string getName() const {
        return supplierName;
    }
 
    void addSuppliedProduct(int productId) {
        if (!suppliesProduct(productId)) {
            suppliedProductIds.push_back(productId);
        } else {
            cout << "[Supplier] Product " << productId
                 << " is already linked to " << supplierName << endl;
        }
    }
 
    void displaySupplier() const {
        cout << "Supplier #" << supplierId << ": " << supplierName
             << " (Contact: " << contactEmail << ")" << endl;
        cout << "  Supplies product IDs: ";
        if (suppliedProductIds.empty()) {
            cout << "none";
        } else {
            for (size_t i = 0; i < suppliedProductIds.size(); i++) {
                cout << suppliedProductIds[i];
                if (i + 1 < suppliedProductIds.size()) cout << ", ";
            }
        }
        cout << endl;
    }
 
    string getEmail() const {
        return contactEmail;
    }
 
    const vector<int>& getSuppliedProducts() const {
        return suppliedProductIds;
    }
 
    bool suppliesProduct(int productId) const {
        return find(suppliedProductIds.begin(), suppliedProductIds.end(), productId)
               != suppliedProductIds.end();
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
        orderedItems.push_back({product, quantity});
        totalAmount += product.getPrice() * quantity;
    }

    void setStatus(string newStatus) {
        status = newStatus;
    }

    void displayOrder() const {
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
// TEAM MEMBER 5: GUI Presentation Layer & Desktop Windows (Win32 API)
// ============================================================================
class GUIController {
private:
    enum { ID_ADD = 1, ID_PLUS, ID_MINUS, ID_PRICE, ID_DELETE, ID_CLOSE };     ////////////////changed
 
    HWND window;
    HWND list;
    HWND nameEdit;
    HWND priceEdit;
    HWND stockEdit;
    HWND supplierEdit;
    vector<Product>& products;
    int nextId;
 
    string getText(HWND edit) const {
        char buf[256];
        GetWindowTextA(edit, buf, sizeof(buf));
        return string(buf);
    }
 
    int selectedIndex() const {
        return ListView_GetNextItem(list, -1, LVNI_SELECTED);
    }
 
    HWND makeControl(const char* cls, const char* text, DWORD style,
                     int x, int y, int w, int h, int id = 0) {
        return CreateWindowExA(0, cls, text, WS_CHILD | WS_VISIBLE | style,
                               x, y, w, h, window, (HMENU)(INT_PTR)id,
                               GetModuleHandleA(NULL), NULL);
    }
 
    void addProduct() {
        string name = getText(nameEdit);
        double price = atof(getText(priceEdit).c_str());
        int stock = atoi(getText(stockEdit).c_str());
        int supplierId = atoi(getText(supplierEdit).c_str()); // Read dynamic supplier ID

        if (name.empty() || price <= 0 || stock < 0 || supplierId <= 0) {
            showAlertPopup("Please enter valid name, price, stock, and supplier ID.");
            return;
        }

        DatabaseManager& db = DatabaseManager::getInstance();
        string sql = "INSERT INTO products (name, quantity, price, category_id, supplier_id) VALUES ('" + 
                     name + "', " + to_string(stock) + ", " + to_string(price) + ", 1, " + to_string(supplierId) + ");";
        
        if (db.executeQuery(sql)) {
            products = db.loadProductsFromDB();
            refreshTable();
            
            SetWindowTextA(nameEdit, "");
            SetWindowTextA(priceEdit, "");
            SetWindowTextA(stockEdit, "");
            SetWindowTextA(supplierEdit, "");
            
            showAlertPopup("Product successfully added to PostgreSQL with Supplier ID #" + to_string(supplierId) + "!");
        } else {
            showAlertPopup("Failed to save product to database.");
        }
    }
 
    void changeStock(int sign) {
        int idx = selectedIndex();
        if (idx < 0) { showAlertPopup("Please select a product in the table first."); return; }
 
        int amount = atoi(getText(stockEdit).c_str());
        if (amount <= 0) { showAlertPopup("Type an amount greater than 0 in the Stock box."); return; }
 
        // Calculate the new stock quantity
        int currentStock = products[idx].getStock();
        int newStock = currentStock + (sign * amount);
        if (newStock < 0) {
            newStock = 0;
        }

        int productId = products[idx].getId();         /////////////////////////////////////

        // Update the PostgreSQL database permanently
        DatabaseManager& db = DatabaseManager::getInstance();
        string sql = "UPDATE products SET quantity = " + to_string(newStock) + 
                     " WHERE id = " + to_string(productId) + ";";

        if (db.executeQuery(sql)) {
            // Reload the product list from the database to keep everything synchronized
            products = db.loadProductsFromDB();
            refreshTable();
            
            // Keep the row selected in the UI
            ListView_SetItemState(list, idx, LVIS_SELECTED | LVIS_FOCUSED,
                                  LVIS_SELECTED | LVIS_FOCUSED);
 
            if (products[idx].getStock() < 10) {
                showAlertPopup("Low Stock Alert: " + products[idx].getName() +
                               " has only " + to_string(products[idx].getStock()) + " units left!");
            } else {
                showAlertPopup("Stock successfully updated in PostgreSQL database!");
            }
        } else {
            showAlertPopup("Failed to update stock in database.");
        }///////////////////////////////////////
    }

    void updatePrice() {                         ////////////////////////Added Function
        int idx = selectedIndex();
        if (idx < 0) { 
            showAlertPopup("Please select a product in the table first."); 
            return; 
        }

        double newPrice = atof(getText(priceEdit).c_str());
        if (newPrice <= 0) { 
            showAlertPopup("Please enter a valid new price greater than 0."); 
            return; 
        }

        int productId = products[idx].getId();

        // Update the price in PostgreSQL permanently
        DatabaseManager& db = DatabaseManager::getInstance();
        string sql = "UPDATE products SET price = " + to_string(newPrice) + 
                     " WHERE id = " + to_string(productId) + ";";

        if (db.executeQuery(sql)) {
            products = db.loadProductsFromDB();
            refreshTable();
            
            // Keep the row selected
            ListView_SetItemState(list, idx, LVIS_SELECTED | LVIS_FOCUSED,
                                  LVIS_SELECTED | LVIS_FOCUSED);

            showAlertPopup("Product price successfully updated in PostgreSQL!");
        } else {
            showAlertPopup("Failed to update price in database.");
        }
    }

    void deleteProduct() {                         ////////////////////////Added Function
        int idx = selectedIndex();
        if (idx < 0) {
            showAlertPopup("Please select a product in the table to delete.");
            return;
        }

        int productId = products[idx].getId();

        // Delete from PostgreSQL database permanently
        DatabaseManager& db = DatabaseManager::getInstance();
        if (db.deleteProductFromDB(productId)) {
            // Reload table from database
            products = db.loadProductsFromDB();
            refreshTable();
            showAlertPopup("Product successfully deleted from PostgreSQL database!");
        } else {
            showAlertPopup("Failed to delete product from database.");
        }
    }
 
    static BOOL CALLBACK applyFont(HWND child, LPARAM) {
        SendMessageA(child, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        return TRUE;
    }
 
    static LRESULT CALLBACK windowProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
        if (msg == WM_NCCREATE) {
            CREATESTRUCTA* cs = (CREATESTRUCTA*)l;
            SetWindowLongPtrA(h, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
            return DefWindowProcA(h, msg, w, l);
        }
        GUIController* self = (GUIController*)GetWindowLongPtrA(h, GWLP_USERDATA);
 
        if (msg == WM_COMMAND && self) {
            switch (LOWORD(w)) {
                case ID_ADD:   self->addProduct();    break;
                case ID_PLUS:  self->changeStock(+1); break;
                case ID_MINUS: self->changeStock(-1); break;
                case ID_CLOSE: DestroyWindow(h);      break;
                case ID_PRICE: self->updatePrice(); break;        //////////////////////
                case ID_DELETE: self->deleteProduct(); break;     //////////////////////
            }
            return 0;
        }
        if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
        return DefWindowProcA(h, msg, w, l);
    }
 
public:
    GUIController(vector<Product>& productList)
        : window(NULL), list(NULL), nameEdit(NULL), priceEdit(NULL),
          stockEdit(NULL), products(productList), nextId(1000) {}
 
    void openMainWindow() {
        cout << "[GUI] Opening Smart Warehouse Desktop Window (Win32)..." << endl;
 
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icc);
 
        WNDCLASSA wc = {};
        wc.lpfnWndProc   = windowProc;
        wc.hInstance     = GetModuleHandleA(NULL);
        wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = "WarehouseWindow";
        RegisterClassA(&wc);
 
        window = CreateWindowExA(0, "WarehouseWindow", "Smart Warehouse System",
                                 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                 CW_USEDEFAULT, CW_USEDEFAULT, 716, 500,
                                 NULL, NULL, wc.hInstance, this);
 
        list = makeControl(WC_LISTVIEWA, "", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER,
                           10, 10, 680, 290);
        ListView_SetExtendedListViewStyle(list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
 
        const char* titles[] = { "ID", "Name", "Price", "Stock" };
        int widths[]         = {  70,   320,    140,     130 };
        for (int i = 0; i < 4; i++) {
            LVCOLUMNA col = {};
            col.mask    = LVCF_TEXT | LVCF_WIDTH;
            col.pszText = (LPSTR)titles[i];
            col.cx      = widths[i];
            SendMessageA(list, LVM_INSERTCOLUMNA, i, (LPARAM)&col);
        }
 
        makeControl("STATIC", "Product name", 0, 10, 315, 120, 18);
        makeControl("STATIC", "Price", 0, 320, 315, 100, 18);
        makeControl("STATIC", "Stock / Amount", 0, 440, 315, 120, 18);
        makeControl("STATIC", "Supplier ID", 0, 560, 315, 100, 18);        ///////////////////////added
        nameEdit  = makeControl("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 10, 335, 300, 24);
        priceEdit = makeControl("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 320, 335, 110, 24);
        stockEdit = makeControl("EDIT", "", WS_BORDER | ES_NUMBER, 440, 335, 110, 24);
        supplierEdit = makeControl("EDIT", "1", WS_BORDER | ES_NUMBER, 560, 335, 80, 24); ///////////////////////added
 
        makeControl("BUTTON", "Add Product",      BS_PUSHBUTTON, 10, 385, 130, 32, ID_ADD);
        makeControl("BUTTON", "Add Stock (+)",    BS_PUSHBUTTON, 150, 385, 130, 32, ID_PLUS);
        makeControl("BUTTON", "Remove Stock (-)", BS_PUSHBUTTON, 290, 385, 150, 32, ID_MINUS);
        makeControl("BUTTON", "Close",            BS_PUSHBUTTON, 590, 385, 100, 32, ID_CLOSE);
        makeControl("BUTTON", "Update Price", BS_PUSHBUTTON, 450, 385, 130, 32, ID_PRICE); ///////////////////////added
        makeControl("BUTTON", "Delete Product", BS_PUSHBUTTON, 450, 425, 130, 32, ID_DELETE);///////////////////////added
 
        EnumChildWindows(window, applyFont, 0);
        refreshTable();
 
        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);
 
        MSG msg;
        while (GetMessageA(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
 
    void refreshTable() {
        ListView_DeleteAllItems(list);
        for (size_t i = 0; i < products.size(); i++) {
            const Product& p = products[i];
            char idText[32], priceText[32], stockText[32];
            snprintf(idText, sizeof(idText), "%d", p.getId());
            snprintf(priceText, sizeof(priceText), "$%.2f", p.getPrice());
            snprintf(stockText, sizeof(stockText), "%d", p.getStock());
 
            string name = p.getName();
            LVITEMA item = {};
            item.mask     = LVIF_TEXT;
            item.iItem    = (int)i;
            item.pszText  = idText;
            ListView_InsertItem(list, &item);
            ListView_SetItemText(list, (int)i, 1, (LPSTR)name.c_str());
            ListView_SetItemText(list, (int)i, 2, priceText);
            ListView_SetItemText(list, (int)i, 3, stockText);
        }
    }
 
    void displayInventoryTable(const vector<Product>&) { refreshTable(); }
 
    void showAlertPopup(string message) {
        cout << "[GUI Alert Popup]: " << message << endl;
        MessageBoxA(window, message.c_str(), "Warehouse Alert", MB_OK | MB_ICONWARNING);
    }
};


// ============================================================================
// MAIN APPLICATION ENTRY POINT (Integration & Testing)
// ============================================================================
int main() {
    cout << "=== Smart Warehouse Inventory & Order Tracking System ===" << endl << endl;

    // 1. Initialize Database Connection & Tables (Leader Yassin)
    DatabaseManager& db = DatabaseManager::getInstance();
    if (db.connect()) {
        db.initializeTables();
    }

    // 2. Test User Authentication (Member 1)
    User clerk(1, "warehouse_clerk", "pass123", "Clerk");
    clerk.displayUser();

    // 3. Load Inventory Dynamically from PostgreSQL Database
    vector<Product> productList = db.loadProductsFromDB();

    // Fallback if database is empty
    if (productList.empty()) {
        productList.push_back(Product(101, "Steel Bracket", 12.50, 200, 1));
        productList.push_back(Product(102, "Conveyor Belt Motor", 450.00, 5, 2));
    }

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
    if (!productList.empty()) {
        myOrder.addProduct(productList[0], 10);
    }
    myOrder.setStatus("Completed");
    myOrder.displayOrder();

    // 6. Test GUI Controller (Member 5)
    GUIController gui(productList);
    gui.showAlertPopup("Warehouse System Initialized Successfully from PostgreSQL!");
    gui.openMainWindow();

    cout << "\nProgram executed successfully." << endl;
    return 0;
}