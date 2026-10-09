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
// CLASS: Category
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

    int getId() const {
        return categoryId;
    }

    string getCategoryName() const {
        return categoryName;
    }
};


// ============================================================================
// CLASS: Product
// ============================================================================
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

    int getCategoryId() const {
        return categoryId;
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
// CLASS: User (holds the Manager / Clerk accounts used by the login screen)
// ============================================================================
class User {
private:
    int userId;
    string username;
    string password;
    string role; // "Manager" or "Clerk"

public:
    User(int id, string uname, string pwd, string r)
        : userId(id), username(uname), password(pwd), role(r) {}

    int getId() const { 
        return userId; 
    }
    string getUsername() const { 
        return username; 
    }
    string getRole() const { 
        return role; 
    }
    bool authenticate(const string& input) const { 
        return password == input; 
    }

    void displayUser() const {
        cout << "User: " << username << " [Role: " << role << "]" << endl;
    }
};


// ============================================================================
// CLASS: Supplier
// ============================================================================
class Supplier {
private:
    int supplierId;
    string supplierName;
    string contactEmail;
    vector<int> suppliedProductIds;

public:
    Supplier(int id, string name, string email)
        : supplierId(id), supplierName(name), contactEmail(email) {}

    int getId() const 
    { 
        return supplierId; 
    }
    string getName() const 
    { 
        return supplierName; 
    }
    string getEmail() const 
    { 
        return contactEmail; 
    }
    const vector<int>& getProductIds() const 
    { 
        return suppliedProductIds; 
    }

    void addSuppliedProduct(int productId) {
        if (find(suppliedProductIds.begin(), suppliedProductIds.end(), productId) == suppliedProductIds.end())
            suppliedProductIds.push_back(productId);
    }

    void displaySupplier() const {
        cout << "Supplier: " << supplierName << " (Contact: " << contactEmail << ")" << endl;
    }
};


// ============================================================================
// CLASS: Order
//   type: "Requested Supply"  = warehouse asked a supplier for stock
//         "Incoming Delivery" = stock that is on its way to the warehouse
// ============================================================================
class Order {
private:
    int orderId;
    string productName;
    string type;
    double unitPrice;
    int quantity;
    string status;    // e.g. "Pending Delivery", "Dispatched", "Delivered"
    string createdAt;

public:
    Order(int id, string name, string orderType, double price, int qty,
          string orderStatus, string time = "")
        : orderId(id), productName(name), type(orderType), unitPrice(price),
          quantity(qty), status(orderStatus), createdAt(time) {}

    int getId() const { 
        return orderId; 
    }
    string getProductName() const { 
        return productName; 
    }
    string getType() const { 
        return type; 
    }
    double getUnitPrice() const { 
        return unitPrice; 
    }
    int getQuantity() const { 
        return quantity; 
    }
    double getTotal() const { 
        return unitPrice * quantity; 
    }
    string getStatus() const { 
        return status; 
    }
    string getCreatedAt() const { 
        return createdAt; 
    }
    bool isIncomingDelivery() const { 
        return type == "Incoming Delivery"; 
    }

    void setStatus(const string& s) { 
        status = s; 
    }

    void displayOrder() const {
        cout << "Order #" << orderId << " | " << productName << " | " << type
             << " | Qty: " << quantity << " | Status: " << status << endl;
    }
};


// ============================================================================
// CLASS: DatabaseManager
// ============================================================================
class DatabaseManager {
private:
    pqxx::connection* conn;
    bool isConnectedStatus;

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
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static DatabaseManager& getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    bool connect() {
        return isConnectedStatus && conn != nullptr && conn->is_open();
    }

    bool executeQuery(const string& sqlQuery) {
        try {
            if (!conn || !conn->is_open()) {
                return false;
            }
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

    void initializeTables() {
        string usersTable = "CREATE TABLE IF NOT EXISTS users ("
                             "id SERIAL PRIMARY KEY, "
                             "username VARCHAR(64) UNIQUE NOT NULL, "
                             "password VARCHAR(64) NOT NULL, "
                             "role VARCHAR(32) NOT NULL);";

        string suppliersTable = "CREATE TABLE IF NOT EXISTS suppliers ("
                                "id SERIAL PRIMARY KEY, "
                                "name VARCHAR(64) NOT NULL, "
                                "email VARCHAR(128) NOT NULL);";

        string productsTable = "CREATE TABLE IF NOT EXISTS products ("
                                "id SERIAL PRIMARY KEY, "
                                "name VARCHAR(64) NOT NULL, "
                                "quantity INT NOT NULL, "
                                "price NUMERIC(10,2) NOT NULL, "
                                "category_id INT, "
                                "supplier_id INT);";

        string ordersTable = "CREATE TABLE IF NOT EXISTS orders ("
                             "id SERIAL PRIMARY KEY, "
                             "product_name VARCHAR(64) NOT NULL, "
                             "type VARCHAR(32) NOT NULL, "
                             "price NUMERIC(10,2) NOT NULL, "
                             "quantity INT NOT NULL, "
                             "status VARCHAR(32) NOT NULL, "
                             "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

        executeQuery(usersTable);
        executeQuery(suppliersTable);
        executeQuery(productsTable);
        executeQuery(ordersTable);

        // First supplier, so the default Supplier ID "1" in the GUI is valid
        executeQuery("INSERT INTO suppliers (name, email) SELECT 'Industrial Parts Ltd', 'sales@indparts.com' "
                     "WHERE NOT EXISTS (SELECT 1 FROM suppliers);");

        cout << "[Database] Warehouse database tables initialized successfully." << endl;
    }

    vector<Product> loadProductsFromDB() {
        vector<Product> dbProducts;
        try {
            if (!conn || !conn->is_open()) {
                return dbProducts;
            }
            
            pqxx::work txn(*conn);
            pqxx::result res = txn.exec("SELECT id, name, price, quantity FROM products ORDER BY id ASC;");
            
            for (auto row : res) {
                int id = row["id"].as<int>();
                string name = row["name"].as<string>();
                double price = row["price"].as<double>();
                int quantity = row["quantity"].as<int>();
                
                dbProducts.push_back(Product(id, name, price, quantity));
            }
        } catch (const exception& e) {
            cerr << "[Load Products Error]: " << e.what() << endl;
        }
        return dbProducts;
    }

    vector<Order> loadOrdersFromDB() {
        vector<Order> out;
        try {
            if (!conn || !conn->is_open()) {
                return out;
            }
            pqxx::work txn(*conn);
            pqxx::result res = txn.exec(
                "SELECT id, product_name, type, price, quantity, status, "
                "to_char(created_at, 'YYYY-MM-DD HH24:MI:SS') AS created "
                "FROM orders ORDER BY id DESC;");
            for (auto row : res) {
                out.push_back(Order(row["id"].as<int>(),
                                    row["product_name"].as<string>(),
                                    row["type"].as<string>(),
                                    row["price"].as<double>(),
                                    row["quantity"].as<int>(),
                                    row["status"].as<string>(),
                                    row["created"].as<string>()));
            }
        } catch (const exception& e) {
            cerr << "[Load Orders Error]: " << e.what() << endl;
        }
        return out;
    }

    vector<Supplier> loadSuppliersFromDB() {
        vector<Supplier> out;
        try {
            if (!conn || !conn->is_open()) {
                return out;
            }
            pqxx::work txn(*conn);
            pqxx::result sres = txn.exec("SELECT id, name, email FROM suppliers ORDER BY id;");
            for (auto row : sres) {
                out.push_back(Supplier(row["id"].as<int>(),
                                       row["name"].as<string>(),
                                       row["email"].as<string>()));
            }

            pqxx::result pres = txn.exec("SELECT id, supplier_id FROM products WHERE supplier_id IS NOT NULL;");
            for (auto row : pres) {
                int supplierId = row["supplier_id"].as<int>();
                for (auto& s : out) {
                    if (s.getId() == supplierId) {
                        s.addSuppliedProduct(row["id"].as<int>());
                    }
                }
            }
        } catch (const exception& e) {
            cerr << "[Load Suppliers Error]: " << e.what() << endl;
        }
        return out;
    }

    bool deleteProductFromDB(int productId) {
        try {
            if (!conn || !conn->is_open()) {
                return false;
            }
            
            pqxx::work txn(*conn);
            string sql = "DELETE FROM products WHERE id = " + to_string(productId) + ";";
            txn.exec(sql);
            
            pqxx::result checkRes = txn.exec("SELECT COUNT(*) FROM products;");
            if (!checkRes.empty() && checkRes[0][0].as<int>() == 0) {
                txn.exec("ALTER SEQUENCE products_id_seq RESTART WITH 1;");
            }
            
            txn.commit();
            return true;
        } catch (const exception& e) {
            cerr << "[Delete Product Error]: " << e.what() << endl;
            return false;
        }
    }
};


// ============================================================================
// CLASS: GUIController (Win32 API Interface)
// ============================================================================
class GUIController {
private:
    enum { 
        ID_TAB = 100, ID_ADD, ID_PLUS, ID_MINUS, ID_PRICE, ID_DELETE, ID_CLOSE
    };
 
    HWND window;
    HWND tabControl;
    HWND inventoryList;
    HWND ordersList;
    HWND nameEdit, priceEdit, stockEdit, supplierEdit;
    vector<Product>& products;
    vector<Supplier>& suppliers;
    string currentUserRole;
 
    string getText(HWND edit) const {
        char buf[256];
        GetWindowTextA(edit, buf, sizeof(buf));
        return string(buf);
    }
 
    HWND makeControl(const char* cls, const char* text, DWORD style,
                     int x, int y, int w, int h, int id = 0) {
        return CreateWindowExA(0, cls, text, WS_CHILD | WS_VISIBLE | style,
                               x, y, w, h, window, (HMENU)(INT_PTR)id,
                               GetModuleHandleA(NULL), NULL);
    }

    void loadOrdersIntoView() {
        ListView_DeleteAllItems(ordersList);

        vector<Order> rows = DatabaseManager::getInstance().loadOrdersFromDB();
        if (rows.empty()) {
            // Sample rows so the tab is never blank (DB offline or no orders yet)
            rows.push_back(Order(0, "Steel Bracket",  "Requested Supply",  12.50,  50, "Pending Delivery", "2026-10-08 10:15:00"));
            rows.push_back(Order(0, "Conveyor Motor", "Incoming Delivery", 450.00, 5,  "Dispatched",       "2026-10-08 12:30:00"));
        }

        for (size_t i = 0; i < rows.size(); i++) {
            const Order& o = rows[i];
            string name = o.getProductName();
            string type = o.getType();
            string status = o.getStatus();
            string time = o.getCreatedAt();
            char priceText[32], qtyText[32];
            snprintf(priceText, sizeof(priceText), "$%.2f", o.getUnitPrice());
            snprintf(qtyText, sizeof(qtyText), "%d", o.getQuantity());

            LVITEMA item = {};
            item.mask = LVIF_TEXT;
            item.iItem = (int)i;
            item.pszText = (LPSTR)name.c_str();
            ListView_InsertItem(ordersList, &item);
            ListView_SetItemText(ordersList, (int)i, 1, (LPSTR)type.c_str());
            ListView_SetItemText(ordersList, (int)i, 2, priceText);
            ListView_SetItemText(ordersList, (int)i, 3, qtyText);
            ListView_SetItemText(ordersList, (int)i, 4, (LPSTR)status.c_str());
            ListView_SetItemText(ordersList, (int)i, 5, (LPSTR)time.c_str());
        }
    }

    void addProduct() {
        string name = getText(nameEdit);
        double price = atof(getText(priceEdit).c_str());
        int stock = atoi(getText(stockEdit).c_str());
        int supplierId = atoi(getText(supplierEdit).c_str());

        if (name.empty() || price <= 0 || stock < 0 || supplierId <= 0) {
            showAlertPopup("Please enter valid name, price, stock, and supplier ID.");
            return;
        }

        bool supplierFound = false;
        for (const Supplier& s : suppliers) {
            if (s.getId() == supplierId) {
                supplierFound = true;
            }
        }
        if (!supplierFound) {
            showAlertPopup("No supplier with that ID exists.");
            return;
        }

        DatabaseManager& db = DatabaseManager::getInstance();
        string sql = "INSERT INTO products (name, quantity, price, category_id, supplier_id) VALUES ('" + 
                     name + "', " + to_string(stock) + ", " + to_string(price) + ", 1, " + to_string(supplierId) + ");";
        
        if (db.executeQuery(sql)) {
            products = db.loadProductsFromDB();
            suppliers = db.loadSuppliersFromDB();
            refreshInventoryTable();
            SetWindowTextA(nameEdit, "");
            SetWindowTextA(priceEdit, "");
            SetWindowTextA(stockEdit, "");
            SetWindowTextA(supplierEdit, "");
            showAlertPopup("Product successfully added to PostgreSQL!");
        } else {
            showAlertPopup("Failed to save product to database.");
        }
    }

    void changeStock(int sign) {
        int idx = ListView_GetNextItem(inventoryList, -1, LVNI_SELECTED);
        if (idx < 0) {
            showAlertPopup("Please select a product in the inventory table first.");
            return;
        }
 
        int amount = atoi(getText(stockEdit).c_str());
        if (amount <= 0) {
            showAlertPopup("Type an amount greater than 0 in the Stock box.");
            return;
        }
 
        int currentStock = products[idx].getStock();
        int newStock = currentStock + (sign * amount);
        if (newStock < 0) {
            newStock = 0;
        }

        int productId = products[idx].getId();
        DatabaseManager& db = DatabaseManager::getInstance();
        string sql = "UPDATE products SET quantity = " + to_string(newStock) + " WHERE id = " + to_string(productId) + ";";

        if (db.executeQuery(sql)) {
            products = db.loadProductsFromDB();
            refreshInventoryTable();
            showAlertPopup("Stock successfully updated in PostgreSQL database!");
        }
    }

    void updatePrice() {
        int idx = ListView_GetNextItem(inventoryList, -1, LVNI_SELECTED);
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
        DatabaseManager& db = DatabaseManager::getInstance();
        string sql = "UPDATE products SET price = " + to_string(newPrice) + " WHERE id = " + to_string(productId) + ";";

        if (db.executeQuery(sql)) {
            products = db.loadProductsFromDB();
            refreshInventoryTable();
            showAlertPopup("Product price successfully updated!");
        }
    }

    void deleteProduct() {
        if (currentUserRole != "Manager") {
            showAlertPopup("Access Denied: Only Managers can delete products.");
            return;
        }

        int idx = ListView_GetNextItem(inventoryList, -1, LVNI_SELECTED);
        if (idx < 0) {
            showAlertPopup("Please select a product in the table to delete.");
            return;
        }

        int productId = products[idx].getId();
        DatabaseManager& db = DatabaseManager::getInstance();
        if (db.deleteProductFromDB(productId)) {
            products = db.loadProductsFromDB();
            suppliers = db.loadSuppliersFromDB();
            refreshInventoryTable();
            showAlertPopup("Product successfully deleted from database!");
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
 
        if (msg == WM_NOTIFY && self) {
            NMHDR* nmhdr = (NMHDR*)l;
            if (nmhdr->idFrom == ID_TAB && nmhdr->code == TCN_SELCHANGE) {
                int sel = TabCtrl_GetCurSel(self->tabControl);
                if (sel == 0) {
                    ShowWindow(self->inventoryList, SW_SHOW);
                    ShowWindow(self->ordersList, SW_HIDE);
                } else if (sel == 1) {
                    ShowWindow(self->inventoryList, SW_HIDE);
                    ShowWindow(self->ordersList, SW_SHOW);
                    self->loadOrdersIntoView();
                }
            }
        }

        if (msg == WM_COMMAND && self) {
            switch (LOWORD(w)) {
                case ID_ADD:   self->addProduct();    break;
                case ID_PLUS:  self->changeStock(+1); break;
                case ID_MINUS: self->changeStock(-1); break;
                case ID_CLOSE: DestroyWindow(h);      break;
                case ID_PRICE: self->updatePrice();   break;
                case ID_DELETE: self->deleteProduct(); break;
            }
            return 0;
        }
        if (msg == WM_DESTROY) { 
            PostQuitMessage(0); 
            return 0; 
        }
        return DefWindowProcA(h, msg, w, l);
    }
 
public:
    GUIController(vector<Product>& productList, vector<Supplier>& supplierList, string role)
        : window(NULL), tabControl(NULL), inventoryList(NULL), ordersList(NULL),
          nameEdit(NULL), priceEdit(NULL), stockEdit(NULL), supplierEdit(NULL),
          products(productList), suppliers(supplierList), currentUserRole(role) {}
 
    void openMainWindow() {
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES };
        if (!InitCommonControlsEx(&icc)) {
            cerr << "[GUI Error] InitCommonControlsEx failed. Error: " << GetLastError() << endl;
        }
 
        WNDCLASSA wc = {};
        wc.lpfnWndProc   = windowProc;
        wc.hInstance     = GetModuleHandleA(NULL);
        wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = "WarehouseWindow";
        
        if (!RegisterClassA(&wc)) {
            DWORD err = GetLastError();
            // ERROR_CLASS_ALREADY_EXISTS (1410) is fine, anything else might be an issue
            if (err != ERROR_CLASS_ALREADY_EXISTS) {
                cerr << "[GUI Error] RegisterClassA failed. Error: " << err << endl;
            }
        }
 
        string windowTitle = "Smart Warehouse System - Logged in as: " + currentUserRole;
        window = CreateWindowExA(
            0, 
            "WarehouseWindow", 
            windowTitle.c_str(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Ensure standard visible overlapping window style
            CW_USEDEFAULT, CW_USEDEFAULT, 720, 520,
            NULL, NULL, wc.hInstance, this
        );

        if (!window) {
            cerr << "[GUI Error] CreateWindowExA failed. Error: " << GetLastError() << endl;
            return;
        }

        // Tab Control Setup
        tabControl = makeControl(WC_TABCONTROLA, "", WS_CHILD | WS_VISIBLE | TCS_TABS, 10, 10, 685, 290, ID_TAB);
        if (!tabControl) {
            cerr << "[GUI Error] Tab control creation failed. Error: " << GetLastError() << endl;
        }

        TCITEMA tie = {};
        tie.mask = TCIF_TEXT;
        tie.pszText = (LPSTR)"Inventory Management (Products)";
        TabCtrl_InsertItem(tabControl, 0, &tie);

        // Add Orders tab ONLY if role is Manager
        if (currentUserRole == "Manager") {
            tie.pszText = (LPSTR)"Orders & Deliveries";
            TabCtrl_InsertItem(tabControl, 1, &tie);
        }
 
        // Inventory List View
        inventoryList = makeControl(WC_LISTVIEWA, "", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | WS_BORDER,
                                    15, 40, 675, 250);
        ListView_SetExtendedListViewStyle(inventoryList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        const char* titles[] = { "ID", "Name", "Price", "Stock" };
        int widths[]         = {  70,   310,    140,     135 };
        for (int i = 0; i < 4; i++) {
            LVCOLUMNA col = {};
            col.mask    = LVCF_TEXT | LVCF_WIDTH;
            col.pszText = (LPSTR)titles[i];
            col.cx      = widths[i];
            ListView_InsertColumn(inventoryList, i, &col);
        }

        // Orders List View (Manager Only Tab - Initially Hidden)
        ordersList = makeControl(WC_LISTVIEWA, "", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_CHILD | WS_BORDER,
                                  15, 40, 675, 250);
        ListView_SetExtendedListViewStyle(ordersList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        const char* orderTitles[] = { "Product Name", "Order Type", "Price", "Quantity", "Status", "Full Timestamp" };
        int orderWidths[]         = {      130,            120,        90,        70,         110,           155 };
        for (int i = 0; i < 6; i++) {
            LVCOLUMNA col = {};
            col.mask    = LVCF_TEXT | LVCF_WIDTH;
            col.pszText = (LPSTR)orderTitles[i];
            col.cx      = orderWidths[i];
            ListView_InsertColumn(ordersList, i, &col);
        }
 
        ShowWindow(ordersList, SW_HIDE);   // makeControl adds WS_VISIBLE, so hide it until the Orders tab is picked

        makeControl("STATIC", "Product name", 0, 15, 315, 120, 18);
        makeControl("STATIC", "Price", 0, 325, 315, 100, 18);
        makeControl("STATIC", "Stock", 0, 435, 315, 80, 18);
        makeControl("STATIC", "Supplier ID", 0, 525, 315, 80, 18);
        
        nameEdit  = makeControl("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 15, 335, 300, 24);
        priceEdit = makeControl("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 325, 335, 95, 24);
        stockEdit = makeControl("EDIT", "", WS_BORDER | ES_NUMBER, 435, 335, 75, 24);
        supplierEdit = makeControl("EDIT", "1", WS_BORDER | ES_NUMBER, 525, 335, 75, 24);
 
        makeControl("BUTTON", "Add Product",      BS_PUSHBUTTON, 15, 385, 120, 32, ID_ADD);
        makeControl("BUTTON", "Add Stock (+)",    BS_PUSHBUTTON, 145, 385, 120, 32, ID_PLUS);
        makeControl("BUTTON", "Remove Stock (-)", BS_PUSHBUTTON, 275, 385, 130, 32, ID_MINUS);
        makeControl("BUTTON", "Update Price",     BS_PUSHBUTTON, 415, 385, 120, 32, ID_PRICE);
        makeControl("BUTTON", "Delete Product",   BS_PUSHBUTTON, 545, 385, 135, 32, ID_DELETE);
        makeControl("BUTTON", "Close App",        BS_PUSHBUTTON, 580, 425, 100, 32, ID_CLOSE);
 
        EnumChildWindows(window, applyFont, 0);
        refreshInventoryTable();
 
        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);
        SetForegroundWindow(window);
        SetFocus(window);
 
        MSG msg;
        while (GetMessageA(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
 
    void refreshInventoryTable() {
        ListView_DeleteAllItems(inventoryList);
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
            ListView_InsertItem(inventoryList, &item);
            ListView_SetItemText(inventoryList, (int)i, 1, (LPSTR)name.c_str());
            ListView_SetItemText(inventoryList, (int)i, 2, priceText);
            ListView_SetItemText(inventoryList, (int)i, 3, stockText);
        }
    }
 
    void showAlertPopup(string message) {
        MessageBoxA(window, message.c_str(), "Warehouse System Notice", MB_OK | MB_ICONINFORMATION);
    }
};


// ============================================================================
// CLASS: LoginController (Win32 role-choice + login screens)
// ============================================================================
class LoginController {
private:
    enum { ID_MANAGER = 200, ID_CLERK, ID_LOGIN, ID_BACK };

    HWND window;
    HWND titleLabel, managerBtn, clerkBtn;
    HWND userLabel, passLabel, userEdit, passEdit, loginBtn, backBtn;
    int screen;              // 0 = choose role, 1 = login form
    string selectedRole;
    bool loggedIn;
    const vector<User>& users;   // accounts that are allowed to log in

    HWND make(const char* cls, const char* text, DWORD style,
              int x, int y, int w, int h, int id = 0) {
        return CreateWindowExA(0, cls, text, WS_CHILD | WS_VISIBLE | style,
                               x, y, w, h, window, (HMENU)(INT_PTR)id,
                               GetModuleHandleA(NULL), NULL);
    }

    string getText(HWND edit) const {
        char buf[256];
        GetWindowTextA(edit, buf, sizeof(buf));
        return string(buf);
    }

    void showRoleScreen() {
        screen = 0;
        selectedRole = "";
        SetWindowTextA(titleLabel, "Welcome! Please choose your role:");
        ShowWindow(managerBtn, SW_SHOW);
        ShowWindow(clerkBtn,   SW_SHOW);
        ShowWindow(userLabel,  SW_HIDE);
        ShowWindow(passLabel,  SW_HIDE);
        ShowWindow(userEdit,   SW_HIDE);
        ShowWindow(passEdit,   SW_HIDE);
        ShowWindow(loginBtn,   SW_HIDE);
        ShowWindow(backBtn,    SW_HIDE);
    }

    void showLoginScreen(const string& role) {
        screen = 1;
        selectedRole = role;
        string t = role + " Login";
        SetWindowTextA(titleLabel, t.c_str());
        ShowWindow(managerBtn, SW_HIDE);
        ShowWindow(clerkBtn,   SW_HIDE);
        ShowWindow(userLabel,  SW_SHOW);
        ShowWindow(passLabel,  SW_SHOW);
        ShowWindow(userEdit,   SW_SHOW);
        ShowWindow(passEdit,   SW_SHOW);
        ShowWindow(loginBtn,   SW_SHOW);
        ShowWindow(backBtn,    SW_SHOW);
        SetWindowTextA(userEdit, "");
        SetWindowTextA(passEdit, "");
        SetFocus(userEdit);
    }

    void tryLogin() {
        string typedUser = getText(userEdit);
        string typedPass = getText(passEdit);

        for (const User& u : users) {
            if (u.getRole() == selectedRole && u.getUsername() == typedUser && u.authenticate(typedPass)) {
                loggedIn = true;
                DestroyWindow(window);
                return;
            }
        }

        MessageBoxA(window, "Incorrect username or password.", "Login Failed", MB_OK | MB_ICONWARNING);
        SetWindowTextA(passEdit, "");
        SetFocus(passEdit);
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
        LoginController* self = (LoginController*)GetWindowLongPtrA(h, GWLP_USERDATA);

        if (msg == WM_COMMAND && self) {
            switch (LOWORD(w)) {
                case ID_MANAGER: self->showLoginScreen("Manager"); break;
                case ID_CLERK:   self->showLoginScreen("Clerk");   break;
                case ID_LOGIN:   self->tryLogin();                 break;
                case ID_BACK:    self->showRoleScreen();           break;
                case IDOK:       if (self->screen == 1) self->tryLogin(); break;      // Enter key
                case IDCANCEL:   if (self->screen == 1) self->showRoleScreen(); break; // Esc key
            }
            return 0;
        }
        if (msg == WM_CLOSE) { DestroyWindow(h); return 0; }
        if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
        return DefWindowProcA(h, msg, w, l);
    }

public:
    LoginController(const vector<User>& userList)
        : window(NULL), titleLabel(NULL), managerBtn(NULL), clerkBtn(NULL),
          userLabel(NULL), passLabel(NULL), userEdit(NULL), passEdit(NULL),
          loginBtn(NULL), backBtn(NULL), screen(0), loggedIn(false), users(userList) {}

    // Shows the role screen, then the login screen. Returns true and fills
    // roleOut ("Manager"/"Clerk") only if the user logged in successfully.
    bool run(string& roleOut) {
        WNDCLASSA wc = {};
        wc.lpfnWndProc   = windowProc;
        wc.hInstance     = GetModuleHandleA(NULL);
        wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = "WarehouseLoginWindow";
        if (!RegisterClassA(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            cerr << "[GUI Error] RegisterClassA (login) failed. Error: " << GetLastError() << endl;
            return false;
        }

        const int clientW = 360, clientH = 260;
        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        RECT r = { 0, 0, clientW, clientH };
        AdjustWindowRect(&r, style, FALSE);
        int winW = r.right - r.left, winH = r.bottom - r.top;
        int x = (GetSystemMetrics(SM_CXSCREEN) - winW) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - winH) / 2;

        window = CreateWindowExA(0, "WarehouseLoginWindow", "Smart Warehouse System",
                                 style, x, y, winW, winH, NULL, NULL, wc.hInstance, this);
        if (!window) {
            cerr << "[GUI Error] CreateWindowExA (login) failed. Error: " << GetLastError() << endl;
            return false;
        }

        titleLabel = make("STATIC", "", SS_CENTER, 20, 25, 320, 24);
        managerBtn = make("BUTTON", "Manager", BS_PUSHBUTTON | WS_TABSTOP, 80,  75, 200, 45, ID_MANAGER);
        clerkBtn   = make("BUTTON", "Clerk",   BS_PUSHBUTTON | WS_TABSTOP, 80, 135, 200, 45, ID_CLERK);

        userLabel = make("STATIC", "Username", 0, 50, 70, 260, 18);
        userEdit  = make("EDIT", "", WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, 50, 90, 260, 24);
        passLabel = make("STATIC", "Password", 0, 50, 125, 260, 18);
        passEdit  = make("EDIT", "", WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD | WS_TABSTOP, 50, 145, 260, 24);
        loginBtn  = make("BUTTON", "Login", BS_DEFPUSHBUTTON | WS_TABSTOP, 50,  195, 125, 32, ID_LOGIN);
        backBtn   = make("BUTTON", "Back",  BS_PUSHBUTTON | WS_TABSTOP,    185, 195, 125, 32, ID_BACK);

        EnumChildWindows(window, applyFont, 0);
        showRoleScreen();

        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);
        SetForegroundWindow(window);

        MSG msg;
        while (GetMessageA(&msg, NULL, 0, 0) > 0) {
            if (!IsDialogMessageA(window, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }

        if (loggedIn) {
            roleOut = selectedRole;
            return true;
        }
        return false;
    }
};


// ============================================================================
// MAIN APPLICATION ENTRY POINT
// ============================================================================
int main() {
    cout << "=== Smart Warehouse Inventory & Order Tracking System ===" << endl;

    DatabaseManager& db = DatabaseManager::getInstance();
    if (db.connect()) {
        db.initializeTables();
    }

    // Accounts allowed to log in (managed by the User class)
    vector<User> users = {
        User(1, "yassin", "123", "Manager"),
        User(2, "gamila", "456", "Clerk")
    };

    // Step 1 + 2: Role choice screen, then login screen (both are real windows)
    LoginController login(users);
    string selectedRole;
    if (!login.run(selectedRole)) {
        cout << "[Login] Cancelled or closed." << endl;
        return 0;
    }
    cout << "[Login Success] Access granted as " << selectedRole << ". Launching main window..." << endl;

    vector<Product> productList = db.loadProductsFromDB();
    if (productList.empty()) {
        productList.push_back(Product(101, "Steel Bracket", 12.50, 200, 1));
        productList.push_back(Product(102, "Conveyor Belt Motor", 450.00, 5, 2));
    }

    vector<Supplier> supplierList = db.loadSuppliersFromDB();

    // Step 3: Main program window (Manager also gets the Orders & Deliveries tab)
    GUIController gui(productList, supplierList, selectedRole);
    gui.openMainWindow();

    return 0;
}