#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cctype>   // for toupper, isdigit

using namespace std;

// ----------------- CONSTANTS -----------------

const int ROWS = 4;                 // A, B, C, D
const int COLS = 6;                 // 1..6
const string TICKET_FILE = "tickets.txt";

// ----------------- STRUCTS -----------------

struct Flight {
    int id;
    string fromCity;
    string toCity;
    double priceFirst;
    double priceBusiness;
    double priceEconomy;
    char seats[ROWS][COLS];   // 'O' = empty, 'X' = booked
};

// ----------------- FUNCTION DECLARATIONS -----------------

void initFlights(Flight flights[], int size);
void loadTicketsAndUpdateSeats(Flight flights[], int size, int &nextTicketId);
int  findFlightIndexById(Flight flights[], int size, int id);

void printMainMenu();
void printFlights(Flight flights[], int size);
void showSeatMap(const Flight &f);
bool parseSeatCode(const string &code, int &rowIndex, int &colIndex);

void saveTicketToFile(int ticketId, int flightId,
                      const string &seatCode, const string &seatClass,
                      double price, const string &name);

void showAllTicketsFromFile(Flight flights[], int size);
void processBooking(Flight flights[], int size, int &nextTicketId);

// ----------------- MAIN FUNCTION -----------------

int main() {
    Flight flights[3];
    initFlights(flights, 3);

    int nextTicketId = 1;

    // Load previous tickets (if any), mark seats as booked
    loadTicketsAndUpdateSeats(flights, 3, nextTicketId);

    while (true) {
        printMainMenu();

        int choice;
        cin >> choice;

        if (!cin) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (choice == 1) {
            // Full flow: select flight -> seat map -> seat -> ticket preview -> confirm
            processBooking(flights, 3, nextTicketId);
        }
        else if (choice == 2) {
            // Exit & Show All Tickets (from file)
            showAllTicketsFromFile(flights, 3);
            cout << "Program ending now. Goodbye.\n";
            break;
        }
        else {
            cout << "Invalid option. Please choose 1 or 2.\n";
        }
    }

    return 0;
}

// ----------------- FUNCTION DEFINITIONS -----------------

// Set up flights and mark all seats as empty
void initFlights(Flight flights[], int size) {
    // Flight 1
    flights[0].id = 101;
    flights[0].fromCity = "Karachi";
    flights[0].toCity = "Islamabad";
    flights[0].priceFirst = 20000;
    flights[0].priceBusiness = 15000;
    flights[0].priceEconomy = 10000;

    // Flight 2
    flights[1].id = 102;
    flights[1].fromCity = "Lahore";
    flights[1].toCity = "Islamabad";
    flights[1].priceFirst = 18000;
    flights[1].priceBusiness = 13000;
    flights[1].priceEconomy = 9000;

    // Flight 3
    flights[2].id = 103;
    flights[2].fromCity = "Karachi";
    flights[2].toCity = "Lahore";
    flights[2].priceFirst = 16000;
    flights[2].priceBusiness = 12000;
    flights[2].priceEconomy = 8000;

    // Initialize all seats as 'O' (empty)
    for (int i = 0; i < size; i++) {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                flights[i].seats[r][c] = 'O';
            }
        }
    }
}

// Find which index in the array has this flight id
int findFlightIndexById(Flight flights[], int size, int id) {
    for (int i = 0; i < size; i++) {
        if (flights[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Read tickets from file, mark seats as booked,
// and figure out what the next ticket ID should be
void loadTicketsAndUpdateSeats(Flight flights[], int size, int &nextTicketId) {
    ifstream in(TICKET_FILE);

    if (!in) {
        // File does not exist yet
        nextTicketId = 1;
        return;
    }

    int maxId = 0;

    int ticketId, flightId;
    string seatCode, seatClass;
    double price;
    string nameRest;   // the passenger name (with spaces)

    // Each line of file:
    // ticketId flightId seatCode seatClass price name...
    while (in >> ticketId >> flightId >> seatCode >> seatClass >> price) {
        // Read rest of the line as the name (it may contain spaces)
        getline(in, nameRest);

        // Track the largest ticketId
        if (ticketId > maxId) {
            maxId = ticketId;
        }

        // Find which flight this ticket belongs to
        int flightIndex = findFlightIndexById(flights, size, flightId);
        if (flightIndex == -1) {
            // Unknown flight, just skip
            continue;
        }

        // Mark the seat as booked in that flight
        int r, c;
        if (parseSeatCode(seatCode, r, c)) {
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                flights[flightIndex].seats[r][c] = 'X';
            }
        }
    }

    in.close();

    // Next ticket ID = maxId + 1 (or 1 if none)
    if (maxId == 0) {
        nextTicketId = 1;
    } else {
        nextTicketId = maxId + 1;
    }
}

void printMainMenu() {
    cout << "\n================= SIMPLE AIRLINE SYSTEM =================\n";
    cout << "1. Select Flight\n";
    cout << "2. Exit Program (Show All Tickets - File Based)\n";
    cout << "=========================================================\n";
    cout << "Enter option (1-2): ";
}

// Print all flights with their basic info
void printFlights(Flight flights[], int size) {
    cout << "\nAvailable Flights:\n";
    cout << "---------------------------------------------------------\n";
    for (int i = 0; i < size; i++) {
        Flight &f = flights[i];
        cout << (i + 1) << ") Flight " << f.id << " : "
             << f.fromCity << " -> " << f.toCity
             << "  [Economy: " << fixed << setprecision(2) << f.priceEconomy
             << ", Business: " << f.priceBusiness
             << ", First: " << f.priceFirst << "]\n";
    }
    cout << "---------------------------------------------------------\n";
}

// Show seat map of one flight
void showSeatMap(const Flight &f) {
    cout << "\nSeat Map for Flight " << f.id << " ("
         << f.fromCity << " -> " << f.toCity << ")\n";

    const int LABEL_WIDTH = 13; // fixed width for "A(First)" etc.

    // Print spaces where row labels go, then column numbers
    for (int i = 0; i < LABEL_WIDTH; i++) {
        cout << ' ';
    }
    for (int c = 1; c <= COLS; c++) {
        cout << setw(3) << c;
    }
    cout << "\n";

    // Row letters
    char rowLetters[ROWS] = { 'A', 'B', 'C', 'D' };

    for (int r = 0; r < ROWS; r++) {
        string cls;

        if (r == 0) cls = "(First)";
        else if (r == 1) cls = "(Business)";
        else cls = "(Economy)";

        // Build full label like "A(First)"
        string label = "";
        label += rowLetters[r];
        label += cls;

        // Print label
        cout << label;

        // Add extra spaces so total width is LABEL_WIDTH
        int spacesToAdd = LABEL_WIDTH - label.size();
        if (spacesToAdd < 1) spacesToAdd = 1; // safety
        for (int s = 0; s < spacesToAdd; s++) {
            cout << ' ';
        }

        // Now print seats in aligned columns
        for (int c = 0; c < COLS; c++) {
            cout << setw(3) << f.seats[r][c];
        }
        cout << "\n";
    }

    cout << "\nLegend: O = Empty, X = Booked\n";
    cout << "Row A = First | Row B = Business | Rows C & D = Economy\n";
}


// Convert seat code like "A1" to rowIndex and colIndex
bool parseSeatCode(const string &code, int &rowIndex, int &colIndex) {
    // We only accept exactly 2 characters, like A1, B3 etc.
    if (code.size() != 2) return false;

    char rowChar = toupper(code[0]); // A, B, C, or D
    char colChar = code[1];          // '1'..'6'

    if (rowChar < 'A' || rowChar > 'A' + ROWS - 1) {
        return false;
    }
    if (!isdigit(colChar)) {
        return false;
    }

    int colNumber = colChar - '0'; // '1' -> 1, '2' -> 2, etc.

    if (colNumber < 1 || colNumber > COLS) {
        return false;
    }

    rowIndex = rowChar - 'A';      // A->0, B->1, ...
    colIndex = colNumber - 1;      // 1->0, 2->1, ...
    return true;
}

// Append a ticket line into the file
void saveTicketToFile(int ticketId, int flightId,
                      const string &seatCode, const string &seatClass,
                      double price, const string &name) {
    ofstream out(TICKET_FILE, ios::app); // append mode

    if (!out) {
        cout << "Error: Could not open ticket file for writing.\n";
        return;
    }

    // Format: ticketId flightId seatCode seatClass price <space> full name
    out << ticketId << " "
        << flightId << " "
        << seatCode << " "
        << seatClass << " "
        << price << " "
        << name << "\n";

    out.close();
}

// Read all tickets from file and show them to user
void showAllTicketsFromFile(Flight flights[], int size) {
    ifstream in(TICKET_FILE);
    cout << "\n================ ALL TICKETS (FILE BASED) ================\n";

    if (!in) {
        cout << "No ticket file found yet. Maybe no tickets were generated.\n";
        return;
    }

    int ticketId, flightId;
    string seatCode, seatClass;
    double price;
    string nameRest;
    bool any = false;

    while (in >> ticketId >> flightId >> seatCode >> seatClass >> price) {
        getline(in, nameRest); // read full name (starts with space)
        if (!nameRest.empty() && nameRest[0] == ' ') {
            nameRest.erase(0, 1); // remove leading space
        }

        any = true;

        // Find flight route
        int flightIndex = findFlightIndexById(flights, size, flightId);
        string fromCity = "Unknown";
        string toCity   = "Unknown";

        if (flightIndex != -1) {
            fromCity = flights[flightIndex].fromCity;
            toCity   = flights[flightIndex].toCity;
        }

        cout << "Ticket ID   : " << ticketId << "\n";
        cout << "Name        : " << nameRest << "\n";
        cout << "Flight      : " << flightId
             << " (" << fromCity << " -> " << toCity << ")\n";
        cout << "Seat        : " << seatCode << " (" << seatClass << " Class)\n";
        cout << "Price       : " << fixed << setprecision(2) << price << "\n";
        cout << "-----------------------------------------------------\n";
    }

    if (!any) {
        cout << "No tickets found in file.\n";
    }

    in.close();
}

// Full booking flow for one ticket
void processBooking(Flight flights[], int size, int &nextTicketId) {
    // 1) Select flight
    printFlights(flights, size);
    cout << "Select flight (1-" << size << "): ";
    int fChoice;
    cin >> fChoice;

    if (fChoice < 1 || fChoice > size) {
        cout << "Invalid flight selection. Returning to main menu.\n";
        return;
    }

    Flight &f = flights[fChoice - 1];

    // 2) Show seat map
    showSeatMap(f);

    // 3) Select seat (loop until valid + not booked)
    string seatInput;
    int r, c;

    while (true) {
        cout << "Enter seat (e.g., A1, B3, C4): ";
        cin >> seatInput;

        if (!parseSeatCode(seatInput, r, c)) {
            cout << "Invalid seat format or out of range. Try again.\n";
            continue;
        }

        if (f.seats[r][c] == 'X') {
            cout << "Seat " << seatInput << " is already booked. Choose another seat.\n";
            continue;
        }

        // Seat is valid and free
        break;
    }

    // 4) Decide seat class and price based on row
    string seatClass;
    double price;

    if (r == 0) {
        seatClass = "First";
        price = f.priceFirst;
    }
    else if (r == 1) {
        seatClass = "Business";
        price = f.priceBusiness;
    }
    else {
        seatClass = "Economy";
        price = f.priceEconomy;
    }

    // 5) Ask passenger full name (with spaces)
    cin.ignore(10000, '\n'); // clear leftover newline
    cout << "Enter passenger full name: ";
    string name;
    getline(cin, name);

    // 6) Show ticket preview
    int ticketId = nextTicketId;  // only increase if user confirms

    cout << "\n========== TICKET PREVIEW ==========\n";
    cout << "Ticket ID   : " << ticketId << "\n";
    cout << "Name        : " << name << "\n";
    cout << "Flight      : " << f.id
         << " (" << f.fromCity << " -> " << f.toCity << ")\n";
    cout << "Seat        : " << seatInput
         << " (" << seatClass << " Class)\n";
    cout << "Price       : " << fixed << setprecision(2) << price << "\n";
    cout << "====================================\n";

    // 7) Ask for confirmation
    cout << "Confirm booking? (Y/N): ";
    char confirm;
    cin >> confirm;
    confirm = toupper(confirm);

    if (confirm == 'Y') {
        // Book the seat
        f.seats[r][c] = 'X';
        nextTicketId++;

        // Save to file
        saveTicketToFile(ticketId, f.id, seatInput, seatClass, price, name);

        cout << "Booking confirmed and saved.\n";
    } else {
        cout << "Booking cancelled. Returning to main menu.\n";
    }
}