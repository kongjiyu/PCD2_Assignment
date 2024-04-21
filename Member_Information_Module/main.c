//inport library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#define MAX_LENGTH 50
#define MAX_PASSWORD_LENGTH 20
#define MAX_VALUE_MEMBER 50

//define sturcture
//voucher struct
struct Voucher {
    char id[5];
    char name[50];
    float discount;
    int price;
};

//member struct
typedef struct {
    char name[50];
    char IC[13];
    char phoneNumber[11];
    char email[30];
    char password[30];
    float wallet;
    int reward_point;
    char card[20];
    struct Voucher voucher[10];
}Member;

//initialize function
//generate menu function
void main_menu();
void login_menu(Member members[], int total_number_member);
int member_information_menu(Member members[], int num_of_mem, int total_number_member);
void reward_menu(Member members[], int num_of_mem);
void member_voucher(Member members[], int num_of_mem);
void buy_voucher(Member members[], int num_of_mem);
void wallet_menu(Member members[], int num_of_mem);
void history_menu(Member members[], int num_of_mem);
void booked_ticket_menu(Member members[], int num_of_mem);
void lost_and_found_menu(Member members[], int num_of_mem);
int delete_acc(Member members[], int num_of_mem, int total_number_member);
int register_menu();
//get user input function
char* get_name();
char* get_IC(Member members[], int num);
char* get_phoneNumber();
char* get_email();
char* get_password();
void getPassword(char* password);

//functional function
int load_members(Member members[]);
int generate_menu(char menu_list[][40], int n_items);
int login_verification(Member members[], int total_number_member);
void remove_spaces(char* str);
char* checkCreditCard();
int is_valid_phone_number(char* phoneNumber);
void update_data(Member members[], int total_number_member);

//main function
void main() {
    main_menu();
}

//let user choose login or register
void main_menu() {
    //list the number of menu
    Member members[MAX_VALUE_MEMBER];
    char log_in_menu[][40] = { "Login", "Register" };
    int total_number_member = load_members(members);
    int choice;
    do {
        choice = generate_menu(log_in_menu, 2);
        switch (choice) {
        case 1:
            //display login menu
            total_number_member = load_members(members);
            login_menu(members, total_number_member);
            break;
            break;
        case 2:
            //display register menu
            choice = register_menu();
            break;
        case -1:
            //exit menu
            choice = -1;
            break;
        default:
            //print error message
            system("cls");
            printf("* * * * * * * * * * * * * * * * * * * *\n");
            printf("*           Invalid Input!            *\n");
            printf("* * * * * * * * * * * * * * * * * * * *\n");
            Sleep(1000);
            system("cls");
            break;
        }
    } while (choice != -1);
}

//login menu
void login_menu(Member members[], int total_number_member) {
    int choice;
    int member_index = login_verification(members, total_number_member);
    if (member_index == -1) {
        printf("Exiting program.\n");
        return; // Exit if login failed or user chose to exit.
    }

    do {
        char menu_list[][40] = { "Train Schedule", "Ticket Booking", "Member Information" };
        choice = generate_menu(menu_list, 3);
        rewind(stdin);
        system("cls");

        switch (choice) {
        case 1:
            //train schedule
            break;
        case 2:
            //ticket booking
            break;
        case 3:
            choice = member_information_menu(members, member_index, total_number_member);
            update_data(members, total_number_member);
            break;
        case -1:
            printf("Logging out...\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            Sleep(1000);
        }
    } while (choice != -1);
}

//member information menu
int member_information_menu(Member members[], int num_of_mem, int total_number_member) {
    int choice = 0;
    do
    {
        char menu_list[][40] = { "Reward Point", "Reload Wallet balance", "Booking History", "Display booked ticket", "Lost and Found", "Delete Account" };
        choice = generate_menu(menu_list, 6);

        rewind(stdin);

        switch (choice) {
        case 1:
            reward_menu(members, num_of_mem);
            break;
        case 2:
            wallet_menu(members, num_of_mem);
            break;
        case 3:
            history_menu(members, num_of_mem);
            break;
        case 4:
            booked_ticket_menu(members, num_of_mem);
            break;
        case 5:
            lost_and_found_menu(members, num_of_mem);
            break;
        case 6:
            choice = delete_acc(members, num_of_mem, total_number_member);
            if (choice == -1) {
                return -1;
            }
            break;
        case -1:
            break;
        default:
            break;
        }
    } while (choice != -1);
    return 0;
}

//reward menu
void reward_menu(Member members[], int num_of_mem) {
    int choice;
    do {
        printf("Name: %s\n", members[num_of_mem].name);
        printf("Reward Point: %d\n", members[num_of_mem].reward_point);
        char menu_list[][40] = { "My Voucher", "Voucher Store" };
        choice = generate_menu(menu_list, 2);
        switch (choice) {
        case 1:
            member_voucher(members, num_of_mem);
            break;
        case 2:
            buy_voucher(members, num_of_mem);

            break;
        case -1:
            break;
        default:
            printf("Invalid input!\n");
            break;
        }
    } while (choice != -1);

}

//let member check his own voucher
void member_voucher(Member members[], int num_of_mem) {
    int count = 0, choice, num = 0;
    for (int i = 0;i < 10;i++) {
        if (strlen(members[num_of_mem].voucher[i].id) > 0) {
            count++;
        }
    }
    do {
        system("cls");
        printf("Total number of Voucher: %d\n", count);
        if (count > 0) {
            printf("* * * * * * * * * * * * *\n");
            printf("* %-20s  *\n", members[num_of_mem].voucher[num].name);
            printf("* * * * * * * * * * * * *\n");
            //temp for menu, wait for ncurses
            printf("Enter 1 to next, -1 to back, 0 to exit > ");
            scanf("%d", &choice);
            rewind(stdin);
            switch (choice) {
            case 1:
                num += 1;
                break;
            case -1:
                if (num == 0) {
                    printf("This is the first Voucher!\n");
                    Sleep(1000);
                    system("cls");
                }
                else {
                    num -= 1;
                }
                break;
            case 0:
                break;
            default:
                printf("Invalid input!\n");
                Sleep(1000);
                system("cls");
                break;
            }

        }
        else {
            printf("No Voucher\n");
            Sleep(1000);
            system("cls");
            choice = 0;
        }
    } while (choice != 0);
}

//let member to buy voucher from store
void buy_voucher(Member members[], int num_of_mem) {
    char choice[5], confirm;
    int check = 0, count = 0;
    for (int i = 0;i < 10;i++) {
        if (strlen(members[num_of_mem].voucher[i].id) > 0) {
            count++;
        }
    }
    struct Voucher voucher[10] = {
        {"V001", "10% Discount Voucher", 0.10, 100},
        {"V002", "20% Discount Voucher", 0.20, 200},
        {"V003", "30% Discount Voucher", 0.30, 300},
        {"V004", "40% Discount Voucher", 0.40, 400},
        {"V005", "50% Discount Voucher", 0.50, 500},
        {"V006", "55% Discount Voucher", 0.55, 600},
        {"V007", "60% Discount Voucher", 0.60, 700},
        {"V008", "65% Discount Voucher", 0.65, 800},
        {"V009", "70% Discount Voucher", 0.70, 900},
        {"V010", "75% Discount Voucher", 0.75, 1000}
    };
    do {
        printf("Voucher Store\n");
        printf("=============\n");
        printf("|%-10s|%-40s|%-10s|\n", "Code", "Voucher Name", "Price");
        printf("===========================================================================\n");
        for (int i = 0; i < 10; i++) {
            printf("|%-10s|%-40s|%-10d|\n", voucher[i].id, voucher[i].name, voucher[i].price);
            printf("================================================================\n");
        }
        printf("Your Reward Point > %d\n", members[num_of_mem].reward_point);
        printf("================================================================\n");
        printf("Enter Voucher that wish to buy(-1 to exit) >");
        scanf("%s", choice);
        rewind(stdin);
        if (strcmp(choice, "-1") == 0) {
            break;
        }
        else {
            for (int i = 0;i < 10;i++) {
                if (strcmp(choice, voucher[i].id) == 0) {
                    check = i;
                }
            }
            if (check == 0) {
                printf("Invalid Code!\n");
            }
            else {
                do {
                    system("cls");
                    printf("Voucher Code: %s\n", voucher[check].id);
                    printf("Voucher Name: %s\n", voucher[check].name);
                    printf("Voucher Price: %d\n", voucher[check].price);
                    printf("Your Reward Points: %d\n", members[num_of_mem].reward_point);
                    printf("Confirm to purchase this voucher?(Y/N) > ");
                    scanf("%c", &confirm);
                    rewind(stdin);
                    if (toupper(confirm) == 'Y') {
                        if (members[num_of_mem].reward_point >= voucher[check].price && count < 10) {
                            members[num_of_mem].reward_point -= voucher[check].price;
                            members[num_of_mem].voucher[count] = voucher[check];
                            system("cls");
                            printf("Purchased Success!\n");
                            Sleep(1000);
                            system("cls");
                        }
                        else {
                            printf("Unable to purchase this voucher! \nThere might be not enough points or amount of voucher exceed maximum limit!\n");
                            Sleep(1000);
                            system("cls");
                        }
                    }
                    else if (toupper(confirm) == 'N') {
                        printf("Purchased Cancelled!\n");
                        Sleep(1000);
                        system("cls");
                    }
                    else {
                        printf("Invalid Input!\n");
                        Sleep(1000);
                        system("cls");
                    }
                } while (toupper(confirm) != 'Y' && toupper(confirm) != 'N');
            }
        }
    } while (strcmp(choice, "-1") != 0);
}

//wallet balance
void wallet_menu(Member members[], int num_of_mem) {
    int choice;
    char menu_list[][40]={"Reload Wallet With Card", "Reload Wallet With Phone Number"};
    float reload_amount;
    do {
        system("cls");
        printf("Name: %s\n", members[num_of_mem].name);
        printf("Wallet Balance: %.2f\n", members[num_of_mem].wallet);
        choice = generate_menu(menu_list, 2);
        rewind(stdin);
        system("cls");
        printf("Name: %s\n", members[num_of_mem].name);
        printf("Wallet Balance: %.2f\n", members[num_of_mem].wallet);
        switch(choice) {
            case 1:
                if (strlen(members[num_of_mem].card) > 0) {
                    // Reload Wallet functionality here
                    printf("Card Number: %s\n", members[num_of_mem].card);
                    printf("Enter amount to reload: ");
                    scanf("%f", &reload_amount);
                    members[num_of_mem].wallet += reload_amount;
                    printf("Wallet reloaded successfully.\n");
                    Sleep(1000);
                } else {
                    // Add Card functionality here
                    printf("Add New Card!\n");
                    strcpy(members[num_of_mem].card , checkCreditCard());
                    if (strcmp(members[num_of_mem].card , "ERROR")==0) {
                        members[num_of_mem].card[0] = '\0';
                        break;
                    }
                    printf("Card added successfully.\n");
                    Sleep(1000);
                    system("cls");
                    printf("Card Number: %s\n", members[num_of_mem].card);
                    printf("Enter amount to reload: ");
                    float reload_amount;
                    scanf("%f", &reload_amount);
                    members[num_of_mem].wallet += reload_amount;
                    printf("Wallet reloaded successfully.\n");
                    Sleep(1000);
                }
                break;
            case 2:
                printf("Phone Number : %s\n", members[num_of_mem].phoneNumber);
                printf("Enter amount to reload: ");
                scanf("%f", &reload_amount);
                if (reload_amount > 1000) {
                    printf("Insufficient Balance!\n");
                    Sleep(1000);
                    system("cls");
                    break;
                }
                members[num_of_mem].wallet += reload_amount;
                printf("Wallet reloaded successfully.\n");
                Sleep(1000);
                break;
            case -1:
                // Exit the menu
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                Sleep(1000);
                system("cls");
        }
    }while(choice != -1);
}
void history_menu(Member members[], int num_of_mem) {

}
void booked_ticket_menu(Member members[], int num_of_mem) {

}
void lost_and_found_menu(Member members[], int num_of_mem) {

}

//delete function
int delete_acc(Member members[], int num_of_mem, int total_number_member) {
    char choice;
    do {
        printf("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");
        printf("*                 Confirm Delete Account ?                *\n");
        printf("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");
        printf("[Y/N] > ");
        scanf("%c", &choice);
        rewind(stdin);
        system("cls");
        if (toupper(choice) != 'Y' && toupper(choice) != 'N') {
            printf("Invalid Input!\n");
            Sleep(1000);
            system("cls");

        }
    } while (toupper(choice) != 'Y' && toupper(choice) != 'N');
    if (toupper(choice) == 'Y') {
        for (int i = num_of_mem;i <= total_number_member;i++) {
            members[i] = members[i + 1];
        }
        FILE* writefptr;
        writefptr = fopen("member.dat", "wb");
        if (writefptr == NULL) {
            printf("Error reading file\n");
            exit(-1);
        }
        for (int i = 0;i < total_number_member;i++) {
            fwrite(members, sizeof(Member), total_number_member, writefptr);
        }

        fclose(writefptr);
        system("cls");
        printf("* * * * * * * * * * * * * * * * * * * *\n");
        printf("*           Account Deleted!          *\n");
        printf("* * * * * * * * * * * * * * * * * * * *\n");
        Sleep(1000);
        system("cls");
        return -1;
    }
    else {
        return 6;
    }
}

//register menu
int register_menu() {
    FILE* fptr;
    FILE* writeptr;
    Member members[MAX_VALUE_MEMBER], temp_member;
    int num_of_member = 0;

    // Read existing members
    fptr = fopen("member.dat", "rb");
    if (fptr == NULL) {
        printf("Error reading file\n");
        Sleep(1000);
        system("cls");
        return 0;
    }

    while (fread(&temp_member, sizeof(Member), 1, fptr) == 1) {
        if (num_of_member < MAX_VALUE_MEMBER) {
            members[num_of_member++] = temp_member;
        }
    }
    fclose(fptr);

    // Get new member details
    strcpy(temp_member.name, get_name());
    strcpy(temp_member.IC, get_IC(members, num_of_member));
    strcpy(temp_member.phoneNumber, get_phoneNumber());
    strcpy(temp_member.email, get_email());
    strcpy(temp_member.password, get_password());

    char choice;
    do {
        system("cls");
        printf("Name: %s\n", temp_member.name);
        printf("IC: %s\n", temp_member.IC);
        printf("Phone Number: %s\n", temp_member.phoneNumber);
        printf("Email: %s\n", temp_member.email);
        printf("Password: ");
        for (int i = 0; i < strlen(temp_member.password); i++) {
            printf("*");
        }
        printf("\nConfirm? (Y/N)\n");
        scanf("%c", &choice);
        rewind(stdin);
        system("cls");
    } while (toupper(choice) != 'Y' && toupper(choice) != 'N');

    if (toupper(choice) == 'Y') {
        if (num_of_member < MAX_VALUE_MEMBER) {
            members[num_of_member] = temp_member;  // Add new member
            members[num_of_member].wallet = 10000;
            members[num_of_member].reward_point = 10000;
            num_of_member++;
        }

        // Reopen file for writing all data
        writeptr = fopen("member.dat", "wb");
        if (writeptr == NULL) {
            printf("Error writing file\n");
            Sleep(1000);
            system("cls");
            return 0;
        }

        // Write all members to file
        fwrite(members, sizeof(Member), num_of_member, writeptr);
        fclose(writeptr);
        system("cls");
        printf("* * * * * * * * * * * * * * * * * * * *\n");
        printf("*           Input recorded!           *\n");
        printf("* * * * * * * * * * * * * * * * * * * *\n");
        Sleep(1000);
        system("cls");
    }

    return 0;
}

//get members data
int load_members(Member members[]) {
    FILE* file = fopen("member.dat", "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file for reading.\n");
        return 0; // Return zero to indicate failure to load any members
    }

    int count = 0;
    while (count < MAX_VALUE_MEMBER && fread(&members[count], sizeof(Member), 1, file) == 1) {
        count++;
    }

    fclose(file);
    return count; // Return the number of members successfully loaded
}

//verification user account
int login_verification(Member members[], int total_number_member) {
    Member temp_member;
    int login_index = -1;
    int found = 0;  // Indicates if the IC was found

    do {
        printf("Enter IC Number [(-1) to exit] > ");
        scanf("%s", temp_member.IC);
        rewind(stdin);
        system("cls");
        if (strcmp(temp_member.IC, "-1") == 0) {
            return -1; // User chooses to exit.
        }

        found = 0;  // Reset found flag for each attempt

        for (int i = 0; i < total_number_member; i++) {
            if (strcmp(temp_member.IC, members[i].IC) == 0) {
                found = 1;  // IC found, now verify password
                printf("Enter Password > ");
                getPassword(temp_member.password);
                system("cls");

                if (strcmp(temp_member.password, members[i].password) == 0) {
                    printf("Login Success!\n");
                    Sleep(1000);
                    system("cls");
                    login_index = i; // Successful login, store index
                    return login_index;  // Exit function successfully
                }
                else {
                    printf("Wrong Password!\n");
                    Sleep(1000);
                    break; // Stop checking other IDs, but continue to allow retry
                }
            }
        }

        // If IC not found, or password was incorrect but no further IDs to check
        if (!found) {
            printf("Wrong ID!\n");
            Sleep(1000);
        }
    } while (login_index == -1);  // Continue until a valid login or user exits

    return login_index;  // Return -1 if exit without login
}

//update data
void update_data(Member members[], int total_number_member) {
    FILE* writefptr;
    writefptr = fopen("member.dat", "wb");
    if (writefptr == NULL) {
        printf("Error reading file\n");
        exit(-1);
    }
    fwrite(members, sizeof(Member), total_number_member, writefptr);
    fclose(writefptr);
}

//get input function
char* get_name() {
    char* name = malloc(MAX_LENGTH * sizeof(char));
    printf("Enter Name > ");
    scanf("%49[^\n]s", name);
    rewind(stdin);
    system("cls");
    return name;
}
char* get_IC(Member members[], int num) {
    int check = 0;
    char* IC = malloc(13 * sizeof(char));
    do {
        check = 0;
        printf("Enter IC Number > ");
        scanf("%s", IC);
        rewind(stdin);
        system("cls");
        for (int i = 0;i < num;i++) {
            if (check == 0) {
                if (strcmp(IC, members[i].IC) == 0) {
                    printf("Account registered!\n");
                    Sleep(1000);
                    system("cls");
                    check = 1;
                }
            }
        }
        if (check == 0) {
            return IC;
        }

    } while (check == 1);
}
char* get_phoneNumber() {
    do
    {
        char * phoneNumber = malloc(12 * sizeof(char));
        printf("Enter Phone Number > +60");
        scanf("%s", phoneNumber);
        rewind(stdin);

        system("cls");
        if (is_valid_phone_number(phoneNumber)) {
            return phoneNumber;
        }
    } while (1);
    
}
char* get_email() {
    char* email = malloc(MAX_LENGTH * sizeof(char));
    printf("Enter Your email > ");
    scanf("%s", email);
    rewind(stdin);
    system("cls");
    return email;
}
char* get_password() {
    do {
        char* password = malloc(100 * sizeof(char)), reconfirm_password[MAX_PASSWORD_LENGTH];
        printf("Enter Password > ");
        getPassword(password);
        system("cls");
        printf("Enter Password Again > ");
        getPassword(reconfirm_password);
        system("cls");
        if (strcmp(password, reconfirm_password) == 0) {
            return password;
        }
        else {
            printf("Unmatch password!\n");
            Sleep(1000);
            system("cls");
        }
    } while (1);


}

//password function
void getPassword(char* password) {
    char c;
    int pos = 0;

    while (1) {
        c = _getch();  // Read a character without echoing it

        // Check for Enter key
        if (c == '\r') {
            password[pos] = '\0';  // Null-terminate the string
            break;
        }
        else if (c == '\b') {  // Check for backspace
            if (pos > 0) {
                printf("\b \b");  // Erase the last '*' character
                pos--;
            }
        }
        else if (pos < MAX_PASSWORD_LENGTH - 1) {  // Check for buffer overflow
            printf("*");  // Print a '*' character
            password[pos++] = c;
        }
    }
}

//generate menu function
int generate_menu(char menu_list[][40], int n_items) {
    //declare choice variable
    int choice;
    //loop to print menu display
    for (int i = 0;i < n_items;i++) {
        printf("* * * * * * * * * * * * * * * * * * * *\n");
        printf("* [%d] %-31s *\n", i + 1, menu_list[i]);
    }
    printf("* * * * * * * * * * * * * * * * * * * *\n");
    printf("* [-1] Exit                           *\n");
    printf("* * * * * * * * * * * * * * * * * * * *\n");
    //get user choice
    printf(" > ");
    scanf("%d", &choice);
    rewind(stdin);
    system("cls");
    return choice;
}

//credit card function
void remove_spaces(char* str) {
    char* temp = str;
    do {
        //when it is blank, go to the next word
        while (*temp == ' ') {
            ++temp;
        }
    } while (*str++ = *temp++);
}
char* checkCreditCard() {
    char* cardNumber;
    cardNumber = (char*)malloc(20);
    int sum = 0, digit, i;

    printf("Enter your credit card number: ");
    fgets(cardNumber, 20, stdin);
    cardNumber[strcspn(cardNumber, "\n")] = 0;
    remove_spaces(cardNumber);
    printf("Card Number: %s\n", cardNumber);

    // Check for card type
    if (cardNumber[0] == '4') {
        printf("Visa\n");
    }
    else if (cardNumber[0] == '5') {
        printf("Mastercard\n");
    }
    else if (cardNumber[0] == '3' && cardNumber[1] == '4' || cardNumber[1] == '7') {
        printf("American Express\n");
    }
    else {
        printf("Unknown card type\n");
    }

    // Check for card legitimacy using Luhn algorithm
    for (i = 0; cardNumber[i] != '\0'; i++) {
        digit = cardNumber[i] - '0';
        if (i % 2 == 0) {
            digit *= 2;
            if (digit > 9) {
                digit = digit % 10 + digit / 10;
            }
        }
        sum += digit;
    }

    if (sum % 10 == 0) {
        return cardNumber;
    }
    else {
        printf("Invalid credit card number\n");
        Sleep(1000);
        system("cls");
        return "ERROR";
    }
}

// Function to check if the phone number is valid
int is_valid_phone_number(char* phoneNumber) {
    int length = strlen(phoneNumber);
    
    // Check total length: should be 10 or 11 digits
    if (length != 9 && length != 10) {
        printf("Not enough length\n");
        return 0;
    }
    
    // Check if all characters are digits
    for (int i = 0; i < length; i++) {
        if (!isdigit(phoneNumber[i])) {
            printf("Not all digit\n");
            return 0;
        }
    }

    // Check if starts with "01"
    if (phoneNumber[0] != '1' ) {
        printf("Not start with 01\n");
        return 0;
    }
    
    if (phoneNumber[1] == '1' && length != 10) {
        return 0;
    }

    if (phoneNumber[1] == '5') {
        return 0;
    }

    if (phoneNumber[1] != '2' && phoneNumber[1] != '3' && phoneNumber[1] != '4' && phoneNumber[1] != '6' && phoneNumber[1] != '7' && phoneNumber[1] != '8' && phoneNumber[1] != '9' && length != 9) {
        return 0;
    }

    // If all checks pass
    return 1;
}
