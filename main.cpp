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
        return supplierId;
    }
 
    string getName() const {
        return supplierName;
    }
 
    // Appends a product ID to the supplied items list
    void addSuppliedProduct(int productId) {
        if (!suppliesProduct(productId)) {
            suppliedProductIds.push_back(productId);
        } else {
            cout << "[Supplier] Product " << productId
                 << " is already linked to " << supplierName << endl;
        }
    }
 
    // Prints vendor contact information and the products they supply
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
    enum { ID_ADD = 1, ID_PLUS, ID_MINUS, ID_CLOSE };
 
    HWND window;
    HWND list;
    HWND nameEdit;
    HWND priceEdit;
    HWND stockEdit;
    vector<Product>& products;
    int nextId;
 
    // ---- helpers -----------------------------------------------------------
    string getText(HWND edit) const {
        char buf[256];
        GetWindowTextA(edit, buf, sizeof(buf));
        return string(buf);
    }
 
    int selectedIndex() const {
        return ListView_GetNextItem(list, -1, LVNI_SELECTED);   // -1 = nothing selected
    }
 
    HWND makeControl(const char* cls, const char* text, DWORD style,
                     int x, int y, int w, int h, int id = 0) {
        return CreateWindowExA(0, cls, text, WS_CHILD | WS_VISIBLE | style,
                               x, y, w, h, window, (HMENU)(INT_PTR)id,
                               GetModuleHandleA(NULL), NULL);
    }
 
    // ---- button actions ----------------------------------------------------
    void addProduct() {
        string name = getText(nameEdit);
        double price = atof(getText(priceEdit).c_str());
        int stock = atoi(getText(stockEdit).c_str());
 
        if (name.empty() || price <= 0 || stock < 0) {
            showAlertPopup("Please enter a name, a price > 0 and a stock amount.");
            return;
        }
        products.push_back(Product(nextId++, name, price, stock));
        refreshTable();
        SetWindowTextA(nameEdit, "");
        SetWindowTextA(priceEdit, "");
        SetWindowTextA(stockEdit, "");
    }
 
    void changeStock(int sign) {
        int idx = selectedIndex();
        if (idx < 0) { showAlertPopup("Please select a product in the table first."); return; }
 
        int amount = atoi(getText(stockEdit).c_str());
        if (amount <= 0) { showAlertPopup("Type an amount greater than 0 in the Stock box."); return; }
 
        products[idx].updateStock(sign * amount);
        refreshTable();
        ListView_SetItemState(list, idx, LVIS_SELECTED | LVIS_FOCUSED,
                              LVIS_SELECTED | LVIS_FOCUSED);   // keep row selected
 
        if (products[idx].getStock() < 10)
            showAlertPopup("Low Stock Alert: " + products[idx].getName() +
                           " has only " + to_string(products[idx].getStock()) + " units left!");
    }
 
    // ---- Windows plumbing --------------------------------------------------
    static BOOL CALLBACK applyFont(HWND child, LPARAM) {
        SendMessageA(child, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        return TRUE;
    }
 
    static LRESULT CALLBACK windowProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
        if (msg == WM_NCCREATE) {   // remember which GUIController owns this window
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
 
    // Opens the window and keeps running until the user closes it
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
 
        // --- table (ListView with 4 columns) ---
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
 
        // --- labels + input boxes ---
        makeControl("STATIC", "Product name", 0, 10, 315, 120, 18);
        makeControl("STATIC", "Price", 0, 320, 315, 100, 18);
        makeControl("STATIC", "Stock / Amount", 0, 440, 315, 120, 18);
        nameEdit  = makeControl("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 10, 335, 300, 24);
        priceEdit = makeControl("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 320, 335, 110, 24);
        stockEdit = makeControl("EDIT", "", WS_BORDER | ES_NUMBER, 440, 335, 110, 24);
 
        // --- buttons ---
        makeControl("BUTTON", "Add Product",      BS_PUSHBUTTON, 10, 385, 130, 32, ID_ADD);
        makeControl("BUTTON", "Add Stock (+)",    BS_PUSHBUTTON, 150, 385, 130, 32, ID_PLUS);
        makeControl("BUTTON", "Remove Stock (-)", BS_PUSHBUTTON, 290, 385, 150, 32, ID_MINUS);
        makeControl("BUTTON", "Close",            BS_PUSHBUTTON, 590, 385, 100, 32, ID_CLOSE);
 
        EnumChildWindows(window, applyFont, 0);   // nicer font on every control
        refreshTable();
 
        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);
 
        MSG msg;
        while (GetMessageA(&msg, NULL, 0, 0) > 0) {   // keeps window alive until closed
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
    GUIController gui(productList);
    gui.showAlertPopup("Low Stock Alert: Conveyor Belt Motor has only 5 units left!");
    gui.openMainWindow();      // runs until the window is closed
    cout << "\nProgram executed successfully." << endl;
    return 0;
}