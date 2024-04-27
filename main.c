#include <ncurses.h> // ncurses for Unix-like systems
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DAYS_IN_MONTH 42 //6 weeks
#define MAX_COACHES 4
#define MAX_ROWS 10
#define MAX_COLS 4

typedef struct{
    int hours, minutes;
}Time;

typedef struct {
    int day, month, year;
}Date;

typedef struct {
    char stationId[4], laneId[4], stationName[50];
}StationInfo;

typedef struct {
    char laneId[4], laneName[50], terminalStationId[2][4];
}LaneInfo;

typedef struct {
    char stationId1[4], stationId2[4];
    int price, duration;
}PriceDuration;

typedef struct {
    char scheduleId[6], laneId[4], trainId[4], departureStationId[4], destinationStationId[4];
    int departureDay;
    Time time;
}LaneSchedule;

typedef struct {
    char trainId[4];
    StationInfo departure, destination;
    char seats[MAX_COACHES][MAX_ROWS][MAX_COLS];
    int seatCount;
    Time departureTime;
    Date date;
}Train;

typedef struct {
    StationInfo departure, destination;
    LaneSchedule schedule;
    int price, duration;
    char seat[3];
    Time departureTime;
    Date date;
}Ticket;

typedef struct {
    char name[50];
    char IC[13];
    char phoneNumber[11];
    char email[30];
    char password[30];
    float wallet;
    int reward_point;
    char card[20];
    Ticket ticket;
}Member;

void displayTime(){
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    // Buffer to hold the formatted time
    char timeOutput[21];

    // Obtain current time
    // time_t is an arithmetic type capable of representing times
    time_t now = time(NULL);

    // Convert time_t to struct tm (local time)
    struct tm *localTime = localtime(&now);

    // Format time into the specified format: "20 Apr 2024 7:00 AM"
    // %d - Day of the month (01 to 31)
    // %b - Abbreviated month name
    // %Y - Year with century
    // %I - Hour (12-hour clock) as a zero-padded decimal number (01 to 12)
    // %M - Minute as a zero-padded decimal number (00 to 59)
    // %p - AM or PM designation
    strftime(timeOutput, sizeof(timeOutput), "%d %b %Y %I:%M %p", localTime);

    // Print the formatted time
    mvprintw(1, xMax - 22,"%s\n", timeOutput);
    refresh();
}

// Function to get user confirmation. Returns true if user confirms (Yes), false otherwise (No).
int getUserConfirmation(char* message) {
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW* confirmWin = newwin(5, 50, (yMax - 5) / 2, (xMax - 50) / 2);
    clear();
    refresh();
    box(confirmWin, 0, 0);
    mvwprintw(confirmWin, 1, 2, "%s", message);
    wrefresh(confirmWin);

    char* choices[] = {"Yes", "No"};
    int confirm = 0;  // 0 for "Yes", 1 for "No"
    int highlight = 0;  // Start by highlighting "Yes"

    keypad(confirmWin, TRUE);  // Enable keyboard input for the window

    int choice = 0;
    while (choice != 10) {
        for (int i = 0; i < 2; i++) {
            if (i == highlight)
                wattron(confirmWin, A_REVERSE);
            mvwprintw(confirmWin, 3, 10 + i * 25, "%s", choices[i]);
            wattroff(confirmWin, A_REVERSE);
        }
        wrefresh(confirmWin);

        choice = wgetch(confirmWin);
        switch (choice) {
            case KEY_RIGHT:
                highlight = 1;  // Move highlight to "No"
                break;
            case KEY_LEFT:
                highlight = 0;  // Move highlight to "Yes"
                break;
            case 10:  // Enter key
                confirm = highlight;  // Confirm the current highlight
                break;
            default:
                break;
        }
    }
    clear();
    refresh();
    delwin(confirmWin);
    return confirm == 0;  // Return true if "Yes" was selected, false otherwise
}

int displayMenu(WINDOW *window, char *choices[], int numOfChoices) {
    displayTime();
    int winHeight, winWidth;
    getmaxyx(window, winHeight, winWidth);  // Get the dimensions of the window

    // Calculate visible choices count (substract 4 for border and possibly other UI elements)
    int visibleChoices = winHeight - 4;  // Adjust as needed for other UI elements
    int startChoice = 0;  // Index of the first visible choice

    int highlight = 0;  // Highlighted choice index
    keypad(window, TRUE);  // Enable keypad input for navigation
    noecho();  // Disable echoing of typed characters

    int input;
    do {
        werase(window);
        box(window, 0, 0);
        wrefresh(window);

        if(startChoice > 0){
            mvwprintw(window, 1, 5, "^");
        }
        if(startChoice < numOfChoices - visibleChoices){
            mvwprintw(window, winHeight - 2, 5, "v");
        }


        // Display only the part of the choices that fits in the window
        for (int i = 0; i < visibleChoices && (startChoice + i) < numOfChoices; i++) {
            int index = startChoice + i;
            if (index == highlight) {
                wattron(window, A_REVERSE);
            }
            mvwprintw(window, i + 2, 1, "%s", choices[index]);
            if (index == highlight) {
                wattroff(window, A_REVERSE);
            }
        }
        wrefresh(window);

        input = wgetch(window);

        switch (input) {
            case KEY_UP:
            case 'w':
                if (highlight > 0) {
                    highlight--;
                    if (highlight < startChoice) {
                        startChoice = highlight;  // Scroll up
                    }
                }
                break;
            case KEY_DOWN:
            case 's':
                if (highlight < numOfChoices - 1) {
                    highlight++;
                    if (highlight >= startChoice + visibleChoices) {
                        startChoice = highlight - visibleChoices + 1;  // Scroll down
                    }
                }
                break;
            case 'q':
                return -1;
            default:
                break;
        }
    } while (input != 10);  // Enter key

    werase(window);
    refresh();
    return highlight;
}

int dayOfWeek(int year, int month, int day) {
    struct tm time = {0};
    time.tm_year = year - 1900;
    time.tm_mon = month;
    time.tm_mday = day;
    mktime(&time);              // Normalize tm_wday
    return time.tm_wday;
}

int daysInMonth(int year, int month) {
    switch(month) {
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
                return 29;
            } else {
                return 28;
            }
        default:
            return 31;
    }
}

void readStationInformation(FILE *trainScheduleFile, StationInfo *stationInfo, int *numberOfStation){
    //variable initialization
    *numberOfStation = 0;
    char buffer[1024];

    //find station information
    while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
        if(strstr(buffer, "//Station Information")){
            fgets(buffer, sizeof(buffer), trainScheduleFile); //remove header
            break;
        }
    }

    //read station information
    while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL && buffer[0] != '\r') {
        sscanf(buffer, "%[^,], %[^,], \"%[^\"]\"", stationInfo[*numberOfStation].stationId, stationInfo[*numberOfStation].laneId, stationInfo[*numberOfStation].stationName);
        (*numberOfStation)++;
    }
}

void readLaneInformation(FILE *trainScheduleFile, LaneInfo *laneInfo, int *numberOfLane){
    //varialbe initialization
    *numberOfLane = 0;
    char buffer[1024];

    //find lane information
    while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
        if (strstr(buffer, "//Lane Information")) {
            fgets(buffer, sizeof(buffer), trainScheduleFile); //remove header
            break;
        }
    }

    //read lane information
    while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL && buffer[0] != '\r') {
        sscanf(buffer, "%[^,], \"%[^\"]\", %[^;];%[^\r]",
               laneInfo[*numberOfLane].laneId,
               laneInfo[*numberOfLane].laneName,
               laneInfo[*numberOfLane].terminalStationId[0],
               laneInfo[*numberOfLane].terminalStationId[1]
        );
        (*numberOfLane)++;
    }
}

void readPriceAndDuration(FILE *trainScheduleFile, PriceDuration *priceAndDuration, int *numberOfPriceAndDuration){
    //variable initialization
    *numberOfPriceAndDuration = 0;
    char buffer[1024];

    //find price and duration information
    while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
        if (strstr(buffer, "//Between station")) {
            fgets(buffer, sizeof(buffer), trainScheduleFile); //remove header
            break;
        }
    }

    //read price and duration information
    while(fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL && buffer[0] != '\r'){
        sscanf(buffer, "%[^,], %[^,], %d, %d",
               priceAndDuration[*numberOfPriceAndDuration].stationId1,
               priceAndDuration[*numberOfPriceAndDuration].stationId2,
               &priceAndDuration[*numberOfPriceAndDuration].price,
               &priceAndDuration[*numberOfPriceAndDuration].duration
        );
        (*numberOfPriceAndDuration)++;
    }
}

void readLaneSchedule(FILE *trainScheduleFile, LaneSchedule *laneSchedule, int *numberOfSchedule){
    *numberOfSchedule = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
        if (strstr(buffer, "//Lane Schedules")) {
            fgets(buffer, sizeof(buffer), trainScheduleFile); //remove header
            break;
        }
    }

    while(fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL && buffer[0] != '\r'){
        sscanf(buffer, "%[^,], %[^,], %[^,], %[^,], %[^,], %d, %d:%d",
               laneSchedule[*numberOfSchedule].scheduleId,
               laneSchedule[*numberOfSchedule].laneId,
               laneSchedule[*numberOfSchedule].trainId,
               laneSchedule[*numberOfSchedule].departureStationId,
               laneSchedule[*numberOfSchedule].destinationStationId,
               &laneSchedule[*numberOfSchedule].departureDay,
               &laneSchedule[*numberOfSchedule].time.hours,
               &laneSchedule[*numberOfSchedule].time.minutes
        );
        (*numberOfSchedule)++;
    }
}

void readTrainSchedule(StationInfo *stationInfo, LaneInfo *laneInfo, PriceDuration *priceAndDuration, LaneSchedule *laneSchedule, int *numberOfStation, int *numberOfLane, int *numberOfPriceAndDuration, int *numberOfSchedule) {
    FILE *trainScheduleFile;
    trainScheduleFile = fopen("trainSchedule.txt", "r");
    if (trainScheduleFile == NULL) {
        printf("Error: unable to open file\n");
        exit(-1);
    }

    readLaneInformation(trainScheduleFile, laneInfo, numberOfLane);
    readStationInformation(trainScheduleFile, stationInfo, numberOfStation);
    readPriceAndDuration(trainScheduleFile, priceAndDuration, numberOfPriceAndDuration);
    readLaneSchedule(trainScheduleFile, laneSchedule, numberOfSchedule);

    fclose(trainScheduleFile);
}

void getInput(char inputs[], char *title){
    clear();
    refresh();
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW *inputWin = newwin(yMax / 2, xMax / 2, yMax / 4, xMax / 4);
    box(inputWin, 0, 0);
    keypad(inputWin, TRUE);

    mvwprintw(inputWin, 1, 2, "%s: ", title);
    echo();
    wgetstr(inputWin, inputs);
    noecho();

    delwin(inputWin);

    clear();
    refresh();
}

void drawCalendar(WINDOW *win, int y, int m, int highlightDay) {
    char *headers[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    time_t rawtime;
    struct tm *timeinfo, currentTime;
    char buffer[80];
    int row, col, day, currentDay, currentMonth, currentYear;

    time(&rawtime);

    currentTime = *localtime(&rawtime);
    currentDay = currentTime.tm_mday;
    currentMonth = currentTime.tm_mon;
    currentYear = currentTime.tm_year + 1900;

    timeinfo = localtime(&rawtime);
    timeinfo->tm_year = y - 1900;
    timeinfo->tm_mon = m;
    timeinfo->tm_mday = 1;
    mktime(timeinfo);



    int startDay = timeinfo->tm_wday;
    int nDays = daysInMonth(y, m);

    // Display month and year
    strftime(buffer, sizeof(buffer), "%b %Y", timeinfo);
    mvwprintw(win, 1, 7, "%s", buffer);

    // Print day headers
    for (int i = 0; i < 7; i++) {
        mvwprintw(win, 3, i * 3 + 2, "%s", headers[i]);
    }

    // Fill the dates
    day = 1;
    for (int i = 0; i < MAX_DAYS_IN_MONTH; i++) {
        row = (i % 7) * 3 + 2;
        col = (i / 7) + 4;
        if (i >= startDay && day <= nDays) {
            if (day == highlightDay) {
                wattron(win, A_REVERSE);
            }
            if(!(y == currentYear && m == currentMonth && day < currentDay)){
                mvwprintw(win, col, row, "%2d", day);
            }
            wattroff(win, A_REVERSE);
            day++;
        }
    }

    wrefresh(win);
}

void normalize_date(struct tm *tm) {
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    mktime(tm);  // Normalize the tm structure
}

int compare_dates(struct tm date1, struct tm date2, int days) {
    date1.tm_mday += days;

    normalize_date(&date1);
    normalize_date(&date2);

    double diff = difftime(mktime(&date1), mktime(&date2));
    if (diff > 0) {
        return 1;  // date1 is later
    } else if (diff < 0) {
        return 0; // date1 is earlier
    }
    return 1; // dates are the same
}

int selectDate(struct tm *date) {
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW *win = newwin(17, 23, (yMax - 17) / 2, (xMax - 23) / 2);
    keypad(win, TRUE);

    int y = date->tm_year + 1900;
    int m = date->tm_mon;
    int d = date->tm_mday;
    int highlightDay = d;

    int done = 0;
    while (!done) {
        //get selected date
        struct tm selectedDate = {.tm_mday = highlightDay,.tm_mon = m,.tm_year = y - 1900};

        //get current time
        time_t currentTime = time(NULL);
        struct tm currentDate = *localtime(&currentTime);

        werase(win);
        box(win, 0, 0);
        drawCalendar(win, y, m, highlightDay);

        int ch = wgetch(win);
        switch(ch) {
            case KEY_UP:
                if(compare_dates(selectedDate, currentDate, -7)){ //if previous week is later / current week
                    if(highlightDay > 7) {  //if not first week of month
                        highlightDay -= 7;
                    }
                }
                break;
            case KEY_DOWN:
                if (highlightDay + 7 < daysInMonth(y, m)) {  //if next week is in month
                    highlightDay += 7;
                }
                break;
            case KEY_LEFT:
                if (highlightDay == 1 || dayOfWeek(y, m, highlightDay) == 0) { //if go previous year/month
                    if(compare_dates(selectedDate, currentDate, -highlightDay + 1)) { //if previous month is later / current month
                        if (m == 0 && compare_dates(selectedDate, currentDate, -highlightDay + 1)) { //if go previous year and previous year is later / current year
                            m = 11;
                            y--;
                        } else {
                            m--;
                        }
                        highlightDay = daysInMonth(y, m);  // Last day of previous month
                    }else{
                        highlightDay--;  // First day of current month
                    }
                } else {
                    if(compare_dates(selectedDate, currentDate, -1)){ //if previous day is later / current day
                        highlightDay--;
                    }
                }
                break;
            case KEY_RIGHT:
                if (highlightDay == daysInMonth(y, m) || dayOfWeek(y, m, highlightDay) == 6) {
                    m = (m + 1) % 12;  // Next month, if current month is December, go to January
                    y += (m == 0) ? 1 : 0;  // Next year, if current month is January, go to next year
                    highlightDay = 1;  // First day of next month
                } else {
                    highlightDay++;
                }
                break;
            case 'q':
                return -1;
            case 10:  // Enter key
                date->tm_year = y - 1900;
                date->tm_mon = m;
                date->tm_mday = highlightDay;
                done = 1;
                break;
            default:
                break;
        }
    }
    return 1;
}

int getDate(Date *date) {
    int status;
    struct tm selectedDate = {0};
    time_t t = time(NULL); //current time
    selectedDate = *localtime(&t);  // Initialize with current time

    status = selectDate(&selectedDate);
    if(status == -1){
        return 0;
    }

    date->year = selectedDate.tm_year + 1900;
    date->month = selectedDate.tm_mon + 1;
    date->day = selectedDate.tm_mday;
    return 1;
}

void draw_time_selection(WINDOW *timeWin, Time time, int highlight) {
    werase(timeWin);  // Clear the window
    box(timeWin, 0, 0);  // Draw a box around the window

    // Display instructions
    mvwprintw(timeWin, 1, 1, "Use arrow keys to change values and switch fields.");
    mvwprintw(timeWin, 2, 1, "Press Enter to confirm.");

    // Highlight and print the hour, minute, and AM/PM fields
    if (highlight == 0) wattron(timeWin, A_REVERSE);
    mvwprintw(timeWin, 3, 1, "%02d", time.hours);
    wattroff(timeWin, A_REVERSE);

    mvwprintw(timeWin, 3, 4, ":");

    if (highlight == 1) wattron(timeWin, A_REVERSE);
    mvwprintw(timeWin, 3, 5, "%02d", time.minutes);
    wattroff(timeWin, A_REVERSE);

    wrefresh(timeWin);
}

int extractStationNumber(char *stationId) {
    int number = 0;
    sscanf(stationId, "S%d", &number);
    return number;
}

int getTime(WINDOW *menu, LaneSchedule *laneSchedule, int numOfSchedule, Ticket *ticket, int currentDepartureDay, PriceDuration *priceDurations, int numOfPriceDurations) {
    int originStationNum = extractStationNumber(ticket->departure.stationId);

    char **choices = malloc(numOfSchedule * sizeof(char *));
    if (!choices) {
        mvwprintw(menu, 1, 1, "Memory allocation failed.");
        wrefresh(menu);
        return 0;
    }

    int count = 0;
    struct tm arrivalTime;
    time_t rawtime = time(NULL);
    struct tm *timeinfo = localtime(&rawtime);

    for (int i = 0; i < numOfSchedule; i++) {
        int laneStartNum = extractStationNumber(laneSchedule[i].departureStationId);
        int laneEndNum = extractStationNumber(laneSchedule[i].destinationStationId);

        // Only consider schedules that pass through the origin station on the correct day
        if (laneStartNum <= originStationNum && originStationNum <= laneEndNum && currentDepartureDay == laneSchedule[i].departureDay) {
            int travelDuration = 0;
            // Calculate the time it takes to reach the origin station from the lane's start
            for (int j = 0; j < numOfPriceDurations; j++) {
                int priceStartNum = extractStationNumber(priceDurations[j].stationId1);
                int priceEndNum = extractStationNumber(priceDurations[j].stationId2);

                if (laneStartNum <= priceStartNum && priceEndNum <= originStationNum) {
                    travelDuration += priceDurations[j].duration;
                }
            }

            // Adjust the departure time by the travel duration to the origin station
            *timeinfo = (struct tm){
                    .tm_hour = laneSchedule[i].time.hours,
                    .tm_min = laneSchedule[i].time.minutes,
                    .tm_sec = 0
            };
            time_t departureTime = mktime(timeinfo);
            departureTime += travelDuration * 60;  // Convert duration to seconds
            localtime_r(&departureTime, &arrivalTime);

            choices[count] = malloc(150);
            if (!choices[count]) {
                for (int k = 0; k < count; k++) free(choices[k]);
                free(choices);
                mvwprintw(menu, 1, 1, "Memory allocation failed.");
                wrefresh(menu);
                return 0;
            }

            snprintf(choices[count], 150, "%s - Arrive by %02d:%02d (%s) from %s to %s",
                     laneSchedule[i].scheduleId, arrivalTime.tm_hour, arrivalTime.tm_min,
                     laneSchedule[i].trainId, laneSchedule[i].departureStationId, laneSchedule[i].destinationStationId);
            count++;
        }
    }

    if (count > 0) {
        int chosen = displayMenu(menu, choices, count);
        if (chosen == -1) {
            for (int i = 0; i < count; i++) free(choices[i]);
            free(choices);
            return 0;
        }

        sscanf(choices[chosen], "%s ", ticket->schedule.scheduleId); // Extract the schedule ID
        ticket->schedule = laneSchedule[chosen];  // Set the selected schedule
        ticket->departureTime.hours = arrivalTime.tm_hour;
        ticket->departureTime.minutes = arrivalTime.tm_min;
    } else {
        mvwprintw(menu, 1, 1, "No available times for this station and day.");
        wrefresh(menu);
    }

    for (int i = 0; i < count; i++) free(choices[i]);
    free(choices);
    return 1;
}





void initSeats(Train* train) {
    for (int i = 0; i < MAX_COACHES; i++) {
        for (int j = 0; j < MAX_ROWS; j++) {
            for (int k = 0; k < MAX_COLS; k++) {
                train->seats[i][j][k] = 'O';
            }
        }
    }
    train->seatCount = 0;
}

// Helper function to check and create a train segment if it does not exist
void ensureTrainSegmentExists(Train *trains, int *numTrains, char *trainId, StationInfo *departure, StationInfo *destination, Time departureTime, Date date) {
    int segmentExists = 0;
    for (int i = 0; i < *numTrains; i++) {
        if (strcmp(trains[i].trainId, trainId) == 0 &&
            strcmp(trains[i].departure.stationId, departure->stationId) == 0 &&
            strcmp(trains[i].destination.stationId, destination->stationId) == 0 &&
            trains[i].departureTime.hours == departureTime.hours &&
            trains[i].departureTime.minutes == departureTime.minutes &&
            trains[i].date.year == date.year &&
            trains[i].date.month == date.month &&
            trains[i].date.day == date.day) {
            segmentExists = 1;
            break;
        }
    }

    if (!segmentExists) {
        Train *newTrain = &trains[*numTrains];
        strcpy(newTrain->trainId, trainId);
        newTrain->departure = *departure;
        newTrain->destination = *destination;
        newTrain->departureTime = departureTime;
        newTrain->date = date;
        initSeats(newTrain);
        (*numTrains)++;
    }
}

// Function to ensure all required train segments exist between two stations
void ensureAllTrainSegments(Train *trains, int *numTrains, Ticket *ticket, StationInfo *stationInfo, int numStations) {
    int startIdx = extractStationNumber(ticket->departure.stationId);
    int endIdx = extractStationNumber(ticket->destination.stationId);
    int step = (startIdx < endIdx) ? 1 : -1;

    StationInfo current, next;
    current = ticket->departure;

    for (int i = startIdx; step > 0 ? i < endIdx : i > endIdx; i += step) {
        // Find next station info
        char nextStationId[5];
        sprintf(nextStationId, "S%d", i + step);
        for (int j = 0; j < numStations; j++) {
            if (strcmp(stationInfo[j].stationId, nextStationId) == 0) {
                next = stationInfo[j];
                break;
            }
        }

        ensureTrainSegmentExists(trains, numTrains, ticket->schedule.trainId, &current, &next, ticket->departureTime, ticket->date);
        current = next; // Move to the next segment
    }
}


void drawSeatInterface(WINDOW* seatwin, Train* train, int coach, int highlightRow, int highlightCol, int highlightConfirm, int totalNumOfTrain, int bookedSeatList[64][3], int bookedSeatListCount, char seat[][MAX_ROWS][MAX_COLS] ){
    wrefresh(seatwin);
    char coachList[MAX_COACHES] = {'A', 'B', 'C', 'D'};
    box(seatwin, 0, 0);
    mvwprintw(seatwin, 1, 3, "Coach %c", coachList[coach]);

    //show arrow when there's available coach on the side
    if(coach > 0){
        mvwprintw(seatwin, 6, 1, "<");
    }
    if(coach < MAX_COACHES - 1){
        mvwprintw(seatwin, 6, 11, ">");
    }
    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            int isHighlighted = highlightRow == i && highlightCol == j && !highlightConfirm;
            wattron(seatwin, isHighlighted ? A_REVERSE : 0);
            if(j < 2){
                mvwprintw(seatwin, i + 2, j + 4, "%c", seat[coach][i][j]);
            }else{
                mvwprintw(seatwin, i + 2, j + 5, "%c", seat[coach][i][j]);
            }
            wattroff(seatwin, isHighlighted ? A_REVERSE : 0);
        }
    }
    wattron(seatwin, highlightConfirm ? A_REVERSE : 0);
    mvwprintw(seatwin, 13, 3, "Confirm");
    wattroff(seatwin, highlightConfirm ? A_REVERSE : 0);
    displayTime();
    wrefresh(seatwin);
}

void seatSelection(WINDOW* seatwin, int* coach, int* highlightRow, int* highlightCol, int* highlightConfirm, char seat[MAX_COACHES][MAX_ROWS][MAX_COLS], int selectedSeat[][3], int *selectedSeatCount) {
    int ch = wgetch(seatwin);

    switch(ch) {
        case KEY_UP:
        case 'w':
            if (*highlightConfirm) {
                *highlightConfirm = 0;
                (*highlightRow)--;
            } else if(*highlightRow == 0){
                *highlightConfirm = 1;
                *highlightRow = MAX_ROWS;
            } else {
                (*highlightRow)--;
            }
            break;
        case KEY_DOWN:
        case 's':
            if (!*highlightConfirm && *highlightRow == MAX_ROWS - 1) {
                *highlightConfirm = 1;
                (*highlightRow)++;
            } else if (!*highlightConfirm) {
                *highlightRow = (*highlightRow + 1) % MAX_ROWS;
            }
            break;
        case KEY_RIGHT:
        case 'd':
            if (!*highlightConfirm) {
                (*highlightCol)++;
                if (*highlightCol >= MAX_COLS) {
                    *highlightCol = 0;
                    wclear(seatwin);
                    if (*coach < MAX_COACHES - 1) (*coach)++;
                }
            }
            break;
        case KEY_LEFT:
        case 'a':
            if (!*highlightConfirm) {
                (*highlightCol)--;
                if (*highlightCol < 0) {
                    *highlightCol = MAX_COLS - 1;
                    if (*coach > 0) (*coach)--;
                    wclear(seatwin);
                }
            }
            break;
        case 'q':
            *highlightConfirm = 2;
            break;
        case 10:  // Enter key
            if (*highlightConfirm) {
                //confirm seat
                if(getUserConfirmation("Confirm?")){
                    for (int i = 0; i < MAX_COACHES; i++) {
                        for (int j = 0; j < MAX_ROWS; j++){
                            for(int k = 0; k < MAX_COLS; k++){
                                if(seat[i][j][k] == 'S'){
                                    //change train seats and add new ticket into member's ticket variable
                                    selectedSeat[*selectedSeatCount][0] = i;
                                    selectedSeat[*selectedSeatCount][1] = j;
                                    selectedSeat[*selectedSeatCount][2] = k;
                                    (*selectedSeatCount)++;
                                    seat[i][j][k] = 'X';
                                }
                            }
                        }
                    }
                    (*highlightConfirm)++;
                }
                break;
            } else {
                if(seat[*coach][*highlightRow][*highlightCol] == 'O'){ //if not selected
                    seat[*coach][*highlightRow][*highlightCol] = 'S';
                }else if(seat[*coach][*highlightRow][*highlightCol] == 'S'){ //if selected
                    seat[*coach][*highlightRow][*highlightCol] = 'O';
                }else{ //if booked
                    mvprintw(2, 2, "This seat has been booked!");
                }
            }
            break;
        default:
            break;
    }
}

void checkSeat(Train *train, int numTrains, Ticket *ticket, StationInfo *stationInfo, int numStations, int bookedSeatList[][3], int *bookedSeatListCount){
    int startIdx = extractStationNumber(ticket->departure.stationId);
    int endIdx = extractStationNumber(ticket->destination.stationId);
    int step = (startIdx < endIdx) ? 1 : -1;

    StationInfo current = ticket->departure;
    StationInfo next;

    // Iterate through each segment between the start and end stations
    for (int i = startIdx; (step > 0 ? i <= endIdx : i >= endIdx) && i >= 0 && i < numStations; i += step) {
        char currentStationId[5];
        char nextStationId[5];
        sprintf(currentStationId, "S%d", i);
        sprintf(nextStationId, "S%d", i + step);

        // Check each train to see if it matches the current segment and time criteria
        for (int j = 0; j < numTrains; j++) {
            if (strcmp(train[j].departure.stationId, currentStationId) == 0 &&
                strcmp(train[j].destination.stationId, nextStationId) == 0 &&
                train[j].departureTime.hours == ticket->schedule.time.hours &&
                train[j].departureTime.minutes == ticket->schedule.time.minutes &&
                train[j].date.year == ticket->date.year &&
                train[j].date.month == ticket->date.month &&
                train[j].date.day == ticket->date.day) {
                // If the train matches the segment and time/date, check for booked seats
                for(int coach = 0; coach < MAX_COACHES; coach++){
                    for(int row = 0; row < MAX_ROWS; row++){
                        for(int col = 0; col < MAX_COLS; col++){
                            if(train[j].seats[coach][row][col] == 'X'){
                                // Record the booked seat
                                bookedSeatList[*bookedSeatListCount][0] = coach;
                                bookedSeatList[*bookedSeatListCount][1] = row;
                                bookedSeatList[*bookedSeatListCount][2] = col;
                                (*bookedSeatListCount)++;
                            }
                        }
                    }
                }

                // Update current to the next station
                for (int k = 0; k < numStations; k++) {
                    if (strcmp(stationInfo[k].stationId, nextStationId) == 0) {
                        current = stationInfo[k];
                        break;  // Break the inner for-loop once the next station is updated
                    }
                }
                break;  // Break after matching a train
            }
        }
    }
}


void createSeatList(int bookedSeatList[][3], int bookedSeatListCount, char seat[MAX_COACHES][MAX_ROWS][MAX_COLS]){
    // Initialize all seats to 'O'
    for (int i = 0; i < MAX_COACHES; i++) {
        for (int j = 0; j < MAX_ROWS; j++) {
            for (int k = 0; k < MAX_COLS; k++) {
                seat[i][j][k] = 'O';
            }
        }
    }

    // Mark the booked seats as 'X'
    for (int l = 0; l < bookedSeatListCount; l++) {
        int coach = bookedSeatList[l][0];
        int row = bookedSeatList[l][1];
        int col = bookedSeatList[l][2];
        if (coach < MAX_COACHES && row < MAX_ROWS && col < MAX_COLS) {
            seat[coach][row][col] = 'X';
        }
    }
}


void getSeat(Train *train, int totalNumOfTrain, Ticket *ticket, int selectedSeat[][3], int *numOfSelectedSeat, StationInfo *stationInfo, int totalNumOfStation) {
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW* seatwin = newwin(15, 13, yMax / 2 - 15 / 2, xMax / 2 - 10 / 2);
    keypad(seatwin, TRUE);

    int bookedSeatList[MAX_COACHES*MAX_ROWS*MAX_COLS][3];
    int bookedSeatListCount = 0;
    char seat[MAX_COACHES][MAX_ROWS][MAX_COLS];
    checkSeat(train, totalNumOfTrain, ticket ,stationInfo, totalNumOfStation, bookedSeatList, &bookedSeatListCount);
    createSeatList(bookedSeatList, bookedSeatListCount, seat);
    int highlightRow = 0, highlightCol = 0, coach = 0, highlightConfirm = 0, numOfTicket = 0;
    do {
        drawSeatInterface(seatwin, &train[coach], coach, highlightRow, highlightCol, highlightConfirm, totalNumOfTrain, bookedSeatList, bookedSeatListCount, seat);
        seatSelection(seatwin, &coach, &highlightRow, &highlightCol, &highlightConfirm, seat, selectedSeat, numOfSelectedSeat);
    } while (highlightConfirm != 2);  // Loop until "quit" or "confirmation"

    // After seat selection, propagate the seat selection across all relevant trains
    int startIdx = extractStationNumber(ticket->departure.stationId);
    int endIdx = extractStationNumber(ticket->destination.stationId);
    int step = (startIdx < endIdx) ? 1 : -1;

    StationInfo current, next;
    current = ticket->departure;

    for (int i = startIdx; step > 0 ? i < endIdx : i > endIdx; i += step) {
        // Find next station info
        char nextStationId[5];
        sprintf(nextStationId, "S%d", i + step);
        for (int j = 0; j < totalNumOfTrain; j++) {
            if (strcmp(train[j].departure.stationId, current.stationId) == 0 && strcmp(train[j].destination.stationId, nextStationId) == 0) {
                for(int k = 0; k < *numOfSelectedSeat; k++){
                    train[j].seats[selectedSeat[k][0]][selectedSeat[k][1]][selectedSeat[k][2]] = 'X';
                }
                next = stationInfo[j];
                break;
            }
        }

        current = next; // Move to the next segment
    }
}

int searchStation(StationInfo *stationInfo, int numOfStation, StationInfo *filteredResult, int *filteredCount, char *place){
    char stationName[50];
    char title[50];
    sprintf(title, "%s station name", place);
    getInput(stationName, title);

    if(strcmp(stationName, "q") == 0){
        return 0;
    }


    for(int i = 0; i < numOfStation; i++){
        if(strstr(stationInfo[i].stationName, stationName) != NULL){
            filteredResult[*filteredCount] = stationInfo[i];
            (*filteredCount)++;
        }
    }

    return 1;
}

int getStation(WINDOW *menu, StationInfo *stationInfo, int numOfStation, char *place, StationInfo *output){
    StationInfo filteredResult[numOfStation];
    int filteredCount = 0, status;

    status = searchStation(stationInfo, numOfStation, filteredResult, &filteredCount, place);
    if(status == 0){
        return 0;
    }

    if (filteredCount == 0) {
        mvwprintw(menu, 1, 1, "No stations found for %s.", place);
        wrefresh(menu);
        wgetch(menu); // Wait for user input
        return 0; // or handle differently
    }

    char **choices = malloc(filteredCount * sizeof(char*));
    if (!choices) {
        perror("Failed to allocate memory for choices");
        return 0; // or handle differently
    }

    for (int i = 0; i < filteredCount; i++) {
        choices[i] = malloc(256); // Adjust size as needed
        if (!choices[i]) {
            // Clean up previously allocated strings
            for (int j = 0; j < i; j++) {
                free(choices[j]);
            }
            free(choices);
            perror("Failed to allocate memory for a choice string");
            return 0; // or handle differently
        }
        snprintf(choices[i], 255, "%s - %s", filteredResult[i].stationId, filteredResult[i].stationName);
    }

    int choice = displayMenu(menu, choices, filteredCount);
    StationInfo selectedStation = filteredResult[choice]; // Direct mapping to filtered results

    // Free memory
    for (int i = 0; i < filteredCount; i++) {
        free(choices[i]);
    }
    free(choices);

    *output = selectedStation;
    return 1;
}

void calculatePriceAndDurationBetweenStations(char* startStationId, char* endStationId, PriceDuration* priceDurations, int numOfPriceDurations, int *price, int *duration) {
    int startNum = extractStationNumber(startStationId);
    int endNum = extractStationNumber(endStationId);
    int step = (startNum < endNum) ? 1 : -1;
    *price = 0, *duration = 0;

    // Determine the direction of the search based on station numbers and accumulate price
    for (int i = startNum; step > 0 ? i < endNum : i > endNum; i += step) {
        for (int j = 0; j < numOfPriceDurations; j++) {
            char currentStationId[5], nextStationId[5];
            sprintf(currentStationId, "S%d", i);
            sprintf(nextStationId, "S%d", i + step);

            if ((strcmp(priceDurations[j].stationId1, currentStationId) == 0 && strcmp(priceDurations[j].stationId2, nextStationId) == 0) ||
                (strcmp(priceDurations[j].stationId2, currentStationId) == 0 && strcmp(priceDurations[j].stationId1, nextStationId) == 0)) {
                *price += priceDurations[j].price;
                *duration += priceDurations[j].duration;
                break;  // Break once the correct segment is found and price added
            }
        }
    }
}
int getDestinationStation(WINDOW *menu, StationInfo *stationInfo, int numOfStation, StationInfo origin, LaneInfo *laneInfo, int numOfLane, PriceDuration *priceAndDuration, int numOfPriceAndDuration, StationInfo *destination) {
    // Filter stations on the same lane
    StationInfo stationList[numOfStation];
    int totalNumOfStation = 0;
    for (int i = 0; i < numOfStation; i++) {
        if (strcmp(stationInfo[i].laneId, origin.laneId) == 0 && strcmp(stationInfo[i].stationId, origin.stationId) != 0) {
            stationList[totalNumOfStation++] = stationInfo[i];
        }
    }

    // Sort stations by stationId for accurate price and duration calculation
    for (int i = 0; i < totalNumOfStation - 1; i++) {
        for (int j = i + 1; j < totalNumOfStation; j++) {
            if (extractStationNumber(stationList[i].stationId) > extractStationNumber(stationList[j].stationId)) {
                StationInfo temp = stationList[i];
                stationList[i] = stationList[j];
                stationList[j] = temp;
            }
        }
    }

    // Allocate choices for menu display
    char **choices = malloc(totalNumOfStation * sizeof(char*));
    if (!choices) {
        perror("Memory allocation failed");
        return 0;
    }

    // Calculate cumulative price and duration from the origin to each station in sequence
    int cumulativePrice = 0, cumulativeDuration = 0;

    for (int i = 0; i < totalNumOfStation; i++) {
        cumulativePrice = 0;  // Reset for each path calculation
        cumulativeDuration = 0;  // Reset for each path calculation

        int stationIndex = extractStationNumber(origin.stationId);
        int targetStationIndex = extractStationNumber(stationList[i].stationId);
        int step = (stationIndex < targetStationIndex) ? 1 : -1;

        // Calculate the cumulative cost from the origin to the current station in the list
        for (int j = stationIndex; step > 0 ? j < targetStationIndex : j > targetStationIndex; j += step) {
            for (int k = 0; k < numOfPriceAndDuration; k++) {
                char currentStationId[5];
                char nextStationId[5];
                sprintf(currentStationId, "S%d", j);
                sprintf(nextStationId, "S%d", j + step);

                if ((strcmp(priceAndDuration[k].stationId1, currentStationId) == 0 && strcmp(priceAndDuration[k].stationId2, nextStationId) == 0) ||
                    (strcmp(priceAndDuration[k].stationId2, currentStationId) == 0 && strcmp(priceAndDuration[k].stationId1, nextStationId) == 0)) {
                    cumulativePrice += priceAndDuration[k].price;
                    cumulativeDuration += priceAndDuration[k].duration;
                    break;
                }
            }
        }

        choices[i] = malloc(256 * sizeof(char));
        if (!choices[i]) {
            for (int j = 0; j < i; j++) free(choices[j]);
            free(choices);
            perror("Memory allocation failed");
            return 0;
        }

        snprintf(choices[i], 255, "%s - %s | Total Price: RM%d, Total Duration: %d mins",
                 stationList[i].stationId, stationList[i].stationName, cumulativePrice, cumulativeDuration);
    }

    // Display menu to select a station
    int choice = displayMenu(menu, choices, totalNumOfStation);
    if(choice == -1){
        for (int i = 0; i < totalNumOfStation; i++) free(choices[i]);
        free(choices);
        return 0;
    }

    // Assign the selected station to the destination output
    *destination = stationList[choice];

    for (int i = 0; i < totalNumOfStation; i++) free(choices[i]);
    free(choices);

    return 1;
}

void createTicket(Ticket *ticket, int *totalNumOfTicket, int numOfSelectedSeat, int selectedSeats[][3]){
    int firstTicketIndex = *totalNumOfTicket;

    // Loop through the number of selected seats to create new ticket
    for (int i = 0; i < numOfSelectedSeat; i++) {
        // Copy the first ticket's data to each new ticket
        Ticket newTicket = ticket[firstTicketIndex]; // Assuming the first ticket is set up correctly elsewhere
        // Set the seat for the new ticket based on selected seats
        sprintf(newTicket.seat, "%c%d%d", selectedSeats[i][0] + 'A', selectedSeats[i][1], selectedSeats[i][2]);
        ticket[*totalNumOfTicket] = newTicket; // Place the new ticket in the array
        (*totalNumOfTicket)++; // Increment the total number of ticket
    }
}

void displayTicketInformation(Ticket *ticket, int totalNumOfTicket, int numOfSelectedSeat) {
    // Clear previous content and create a border
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW* win = newwin(yMax - 5, xMax - 2, 3, 1);
    box(win, 0, 0);

    char seats[100] = "";

    for(int i = totalNumOfTicket - numOfSelectedSeat; i < totalNumOfTicket; i++){
        // if it is the first seat, don't append a comma before it
        if (i > totalNumOfTicket - numOfSelectedSeat) {
            sprintf(&seats[strlen(seats)], ", %s", ticket[i].seat);
        } else {
            sprintf(seats, "%s", ticket[i].seat);
        }
    }

    // Title
    mvwprintw(win, 1, 1, "Confirmed Ticket Details:");

    // Loop through the ticket array and display each ticket's details
    for (int i = totalNumOfTicket - numOfSelectedSeat; i < totalNumOfTicket; i++) {
        mvwprintw(win, 1, 1, "Ticket #%d:", i + 1);
        mvwprintw(win, 2, 1, "Departure: %s (%s)", ticket[i].departure.stationName, ticket[i].departure.stationId);
        mvwprintw(win, 3, 1, "Destination: %s (%s)", ticket[i].destination.stationName, ticket[i].destination.stationId);
        mvwprintw(win, 4, 1, "Seat: %s", seats);
        mvwprintw(win, 5, 1, "Date: %02d-%02d-%04d", ticket[i].date.day, ticket[i].date.month, ticket[i].date.year);
        mvwprintw(win, 6, 1, "Departure Time: %02d:%02d", ticket[i].departureTime.hours, ticket[i].departureTime.minutes);
        mvwprintw(win, 7, 1, "Total Price: RM%d", ticket[i].price);
        mvwprintw(win, 8, 1, "Total Duration: %d mins", ticket[i].duration);
    }
    mvwprintw(win, yMax-7, 1, "Press any key to continue...");
    // Refresh window to display changes
    wrefresh(win);
    getch();
    clear();
    refresh();
}

void readTicketData(Train *train, Ticket *ticket, int *totalNumOfTrain, int *totalNumOfTicket){
    FILE *ticketBookingFile;
    ticketBookingFile = fopen("TicketBooking.dat", "rb");
    if(ticketBookingFile == NULL){
        printf("Error opening file\n");
        exit(-1);
    }
    fread(totalNumOfTrain, sizeof(int), 1, ticketBookingFile);
    fread(totalNumOfTicket, sizeof(int), 1, ticketBookingFile);
    fread(train, sizeof(Train), *totalNumOfTrain, ticketBookingFile);
    fread(ticket, sizeof(Ticket), *totalNumOfTicket, ticketBookingFile);

    fclose(ticketBookingFile);
}

int storeTicketData(Train *train, Ticket *ticket, int *totalNumOfTrain, int *totalNumOfTicket) {
    FILE *ticketBookingFile = fopen("TicketBooking.dat", "wb");
    if (ticketBookingFile == NULL) {
        printf("Error opening file\n");
        return -1; // Indicate file opening failed
    }

    // Check each fwrite for errors
    fwrite(totalNumOfTrain, sizeof(int), 1, ticketBookingFile);
    fwrite(totalNumOfTicket, sizeof(int), 1, ticketBookingFile);
    fwrite(train, sizeof(Train), *totalNumOfTrain, ticketBookingFile);
    fwrite(ticket, sizeof(Ticket), *totalNumOfTicket, ticketBookingFile);

    fclose(ticketBookingFile);
    return 0; // Success
}

int buyTicket(WINDOW *menu){
    //read ticket booking information
    int totalNumOfTrain, totalNumOfTicket;
    Train train[10000];
    Ticket ticket[10000];
    readTicketData(train, ticket, &totalNumOfTrain, &totalNumOfTicket);


    //get schedule information
    int numOfStation, numOfLane, numOfPriceAndDuration, numOfSchedule;
    StationInfo stationInfo[50];
    LaneInfo laneInfo[10];
    PriceDuration priceAndDuration[100];
    LaneSchedule laneSchedule[1000];
    readTrainSchedule(stationInfo, laneInfo, priceAndDuration, laneSchedule, &numOfStation, &numOfLane, &numOfPriceAndDuration, &numOfSchedule);
    /* TODO:
        1. Get origin station
        2. Get destination station
        3. Get date
        4. Choose time
        - Check weekday or weekend and display time from train schedule struct
        5. Create train
        - Check whether the train is exist for the selected date and time
        - If not exist, create new train struct from origin to destination
        6. Choose seat
        - check id of origin station and destination station
        - Loop through train struct to check whether the place is available for seat
        - if there is any X when loop, then it should display X
        - If train don't have seat, then empty seat for the train.
        7. Calculate price
        - loop through price and duration to calculate price
        8. confirm
    */
    int status;
    status = getStation(menu, stationInfo, numOfStation, "Origin", &ticket[totalNumOfTicket].departure);
    if(status == 0){
        return status;
    }

    status = getDestinationStation(menu, stationInfo, numOfStation, ticket[totalNumOfTicket].departure, laneInfo, numOfLane, priceAndDuration, numOfPriceAndDuration, &ticket[totalNumOfTicket].destination);
    if(status == 0){
        return status;
    }
    calculatePriceAndDurationBetweenStations(ticket[totalNumOfTicket].departure.stationId, ticket[totalNumOfTicket].destination.stationId,priceAndDuration,numOfPriceAndDuration, &ticket[totalNumOfTicket].price, &ticket[totalNumOfTicket].duration);

    clear();
    refresh();
    status = getDate(&ticket[totalNumOfTicket].date);
    if(status == 0){
        return status;
    }

    clear();
    refresh();
    int departureDay = dayOfWeek(ticket[totalNumOfTicket].date.year, ticket[totalNumOfTicket].date.month, ticket[totalNumOfTicket].date.day);
    if(departureDay == 6 || departureDay == 0){
        departureDay = 7;
    }else{
        departureDay = 5;
    }
    status = getTime(menu, laneSchedule, numOfSchedule, &ticket[totalNumOfTicket] ,departureDay, priceAndDuration, numOfPriceAndDuration );
    if(status == 0){
        return status;
    }
    clear();
    refresh();

    int selectedSeat[MAX_COACHES*MAX_COLS*MAX_ROWS][3];
    int numOfSelectedSeat = 0;
    ensureAllTrainSegments(train, &totalNumOfTrain, &ticket[totalNumOfTicket], stationInfo, numOfStation);
    getSeat(train, totalNumOfTrain, &ticket[totalNumOfTicket], selectedSeat, &numOfSelectedSeat, stationInfo, numOfStation);
    if(numOfSelectedSeat == 0){
        return 0;
    }
    ticket[totalNumOfTicket].price = numOfSelectedSeat * ticket[totalNumOfTicket].price;

    createTicket(ticket, &totalNumOfTicket, numOfSelectedSeat, selectedSeat);
    displayTicketInformation( ticket, totalNumOfTicket, numOfSelectedSeat);

    storeTicketData(train, ticket, &totalNumOfTrain, &totalNumOfTicket);
    return 1;
}

void displayAllTicket() {
    Ticket ticket[10000];
    Train train[1000];
    int numOfTicket, numOfTrain;
    readTicketData(train, ticket, &numOfTrain, &numOfTicket);

    WINDOW *displayWin;
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    displayWin = newwin(yMax - 2, xMax - 2, 1, 1);

    int row = yMax - 2;
    int col = xMax - 2;
    // Determine the number of tickets that can be displayed per page
    int ticketsPerPage = (row - 4) / 5;  // Assuming each ticket takes 5 rows to display
    int currentPage = 0;
    int totalPages = (numOfTicket + ticketsPerPage - 1) / ticketsPerPage;

    keypad(displayWin, TRUE);  // Enable keyboard interaction for the menu
    int ch;
    do {
        werase(displayWin);  // Clear the menu window
        box(displayWin, 0, 0);  // Draw a box around the window

        // Display tickets for the current page
        int startTicket = currentPage * ticketsPerPage;
        int endTicket = startTicket + ticketsPerPage < numOfTicket ? startTicket + ticketsPerPage : numOfTicket;

        for (int i = startTicket; i < endTicket; i++) {
            int line = 1 + (i - startTicket) * 5;
            mvwprintw(displayWin, line, 1, "Ticket %d: %s to %s", i + 1, ticket[i].departure.stationName, ticket[i].destination.stationName);
            mvwprintw(displayWin, line + 1, 1, "Date: %02d-%02d-%04d Time: %02d:%02d", ticket[i].date.day, ticket[i].date.month, ticket[i].date.year, ticket[i].departureTime.hours, ticket[i].departureTime.minutes);
            mvwprintw(displayWin, line + 2, 1, "Seat: %s", ticket[i].seat);
            mvwprintw(displayWin, line + 3, 1, "----------------------------------------");
        }

        // Display navigation instructions
        mvwprintw(displayWin, row - 3, 1, "Page %d of %d", currentPage + 1, totalPages);
        if (currentPage > 0) {
            mvwprintw(displayWin, row - 2, 1, "Press 'left arrow' for previous page.");
        }
        if (currentPage < totalPages - 1) {
            mvwprintw(displayWin, row - 2, 20, "Press 'right arrow' for next page.");
        }
        mvwprintw(displayWin, row - 2, col - 20, "Press 'q' to quit.");

        wrefresh(displayWin);  // Refresh the menu window to show the current page
        ch = wgetch(displayWin);  // Wait for user input

        // Navigate through pages
        if (ch == KEY_RIGHT && currentPage < totalPages - 1) {
            currentPage++;
        } else if (ch == KEY_LEFT && currentPage > 0) {
            currentPage--;
        }
    } while (ch != 'q');  // Quit the viewer

    clear();
    refresh();
}

void addLaneSchedule(){
    //initialize newSchedule with default values
    LaneSchedule newSchedule = { "SS000", "", "", "", "", {0}, {0, 0} };
    int laneSelection;
    int stationSelection;
    char confirm;
}

void staff(WINDOW *menu){

}

void memberRegister(WINDOW *menu){

}

void displayLaneSchedule() {
    // Dummy data read function assuming it populates the number of schedules and other details
    LaneInfo laneInfo[500];
    StationInfo stationInfo[500];
    PriceDuration priceAndDuration[500];
    LaneSchedule laneSchedules[500];
    int numberOfSchedule = 0, numberOfLane = 0, numberOfStation = 0, numberOfPrice = 0;

    readTrainSchedule(stationInfo, laneInfo, priceAndDuration, laneSchedules, &numberOfStation, &numberOfLane, &numberOfPrice, &numberOfSchedule);

    if (numberOfSchedule == 0) {
        printw("No schedules available.");
        refresh();
        getch();
        return;
    }

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW* displayWin = newwin(yMax - 2, xMax - 2, 1, 1);
    keypad(displayWin, TRUE);
    int linesPerPage = yMax - 10;
    int currentPage = 0;
    int totalPages = (numberOfSchedule / linesPerPage) + (numberOfSchedule % linesPerPage != 0);
    char departureName[20], destinationName[20], departureOutput[30];

    int ch;
    do {
        werase(displayWin);
        box(displayWin, 0, 0);

        mvwprintw(displayWin, 1, 1, "Lane Schedules Page %d of %d", currentPage + 1, totalPages);
        for(int i = 1; i < xMax-1; i++){
            mvwprintw(displayWin, 2, i, "=");
        }
        mvwprintw(displayWin, 3, 1, "|ID    |Lane  |Train |Departure Station Name |Destination Station Name  |Departure Day   |Time     |");
        for(int i = 1; i < xMax-1; i++){
            mvwprintw(displayWin, 4, i, "=");
        }
        int start = currentPage * linesPerPage;
        int end = start + linesPerPage < numberOfSchedule ? start + linesPerPage : numberOfSchedule;
        for (int i = start; i < end; i++) {
            for(int j = 0; j < numberOfStation; j++){
                if(strcmp(laneSchedules[i].departureStationId, stationInfo[j].stationId) == 0){
                    strcpy(departureName, stationInfo[j].stationName);
                }
                if(strcmp(laneSchedules[i].destinationStationId, stationInfo[j].stationId) == 0){
                    strcpy(destinationName, stationInfo[j].stationName);
                }
            }
            if(laneSchedules[i].departureDay == 5){
                strcpy(departureOutput, "Monday - Friday");
            }else{
                strcpy(departureOutput, "Monday - Sunday");
            }
            mvwprintw(displayWin, 5 + (i - start), 1, "|%-4s |%-5s |%-5s |%-22s |%-25s |%-15s |%02d:%02d    |",
                      laneSchedules[i].scheduleId, laneSchedules[i].laneId, laneSchedules[i].trainId,
                      departureName, destinationName,
                      departureOutput, laneSchedules[i].time.hours, laneSchedules[i].time.minutes);
        }

        mvwprintw(displayWin, 6 + linesPerPage, 1, "Use LEFT/RIGHT to scroll pages. Press 'q' to quit.");
        wrefresh(displayWin);

        ch = wgetch(displayWin);
        switch(ch) {
            case KEY_LEFT:
                if (currentPage > 0) currentPage--;
                break;
            case KEY_RIGHT:
                if (currentPage < totalPages - 1) currentPage++;
                break;
        }
    } while (ch != 'q');

    clear();
    refresh();
    delwin(displayWin);
}

void memberMenu(WINDOW *menu) {
    char *choices[] = {
            "Buy Ticket",
            "Cancel booked ticket (within 3 days before departure)",
            "Modify booked ticket seats (within 3 days before departure)",
            "Display all ticket",
            "Train Schedule",
            "Check information",
            "Log Out"
    };
    int numOfChoice = sizeof(choices) / sizeof(char *);
    int choice, status;

    do{
        choice = displayMenu(menu, choices, numOfChoice);
        clear();
        refresh();
        switch (choice) {
            case 0:
                status = buyTicket(menu);
                if(status){
                    mvwprintw(menu, 1, 1, "Ticket booked successfully.");
                }
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                displayAllTicket(menu);
                break;
            case 4:
                displayLaneSchedule(menu);
                break;
            default:
                break;
        }
    }while(choice != 999);
}

void memberLogin(WINDOW *menu) {
    char username[50];
    char password[50];
    int yMax, xMax;
    int ch, i = 0;

    clear();
    refresh();
    getmaxyx(stdscr, yMax, xMax);
    WINDOW *loginWin = newwin(10, 40, (yMax - 10) / 2, (xMax - 40) / 2);

    box(loginWin, 0, 0);
    keypad(loginWin, TRUE); // Enable keypad for function keys
    echo();

    // Prompt for username
    mvwprintw(loginWin, 2, 2, "Enter username: ");
    wgetstr(loginWin, username);

    // Prompt for password
    mvwprintw(loginWin, 4, 2, "Enter password: ");
    noecho(); // Do not echo the characters on screen
    wrefresh(loginWin);

    while ((ch = wgetch(loginWin)) != '\n' && i < sizeof(password) - 1) {
        if (ch == KEY_BACKSPACE || ch == 127) {  // Handle backspace (127 for ASCII DEL)
            if (i > 0) {
                i--;
                waddch(loginWin, '\b'); // Go back one space
                waddch(loginWin, ' ');  // Erase the star
                waddch(loginWin, '\b'); // Go back one space again
            }
        } else {
            password[i++] = ch;
            waddch(loginWin, '*'); // Display star for each character
        }
    }
    password[i] = '\0'; // Null-terminate the password string

    // Clear window after input
    werase(loginWin);
    box(loginWin, 0, 0);
    mvwprintw(loginWin, 5, 2, "Login successful");
    wrefresh(loginWin);
    getch();  // Wait for key press

    // Clean up
    delwin(loginWin);
    clear();
    refresh();
    memberMenu(menu);
}

void member(WINDOW *menu){
    char *choices[] = {
            "Login",
            "Register",
            "Exit"
    };
    int numOfChoice = sizeof(choices) / sizeof(char *);
    int choice;

    do{
        choice = displayMenu(menu, choices, numOfChoice);
        switch (choice) {
            case 0:
                memberLogin(menu);
                break;
            case 1:
                memberRegister(menu);
                break;
            case 2:
                choice = 999;
                break;
            default:
                break;
        }
    }while(choice != 999);
}

void mainMenu(){
    //get window maximum size
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    //initialize choices
    char *choices[] = {
            "Staff",
            "Member",
            "Exit"
    };
    int numOfChoices = sizeof(choices) / sizeof(char *);
    int choice;

    //create window
    WINDOW *menu = newwin(10, xMax - 2, yMax - 10, 1);

    //loop for menu
    do{
        choice = displayMenu(menu, choices, numOfChoices);

        //clear menu and proceed
        clear();
        refresh();

        //proceed to the next function
        switch (choice) {
            case 0:
                staff(menu);
                break;
            case 1:
                member(menu);
                break;
            case 2: //exit
                choice = 999;
                break;
            default:
                break;
        }
    }while(choice != 999);
}

void createTrainforTextFile() {
    FILE *ticketBookingFile = fopen("TicketBooking.dat", "wb");
    if (ticketBookingFile == NULL) {
        printf("Error opening file\n");
        return;
    }

    int numberOfTrain = 1, numberOfTicket = 2;
    Train *train = (Train*) malloc(sizeof(Train) * numberOfTrain);
    Ticket *ticket = (Ticket*) malloc(sizeof(Ticket) * numberOfTicket);

    Ticket ticket1 = {
            {"S1", "L1", "Huat Ah"},
            {"S2", "L1", "Gokfa"},
            {"SS001", "L1", "T1", "S1", "S9", 7, {6, 0}},
            10, 23, "A00",
            {6, 0},
            {26, 4, 2024}
    };
    Ticket ticket2 = {
            {"S1", "L1", "Huat Ah"},
            {"S2", "L1", "Gokfa"},
            {"SS001", "L1", "T1", "S1", "S9", 7, {6, 0}},
            10, 23, "A01",
            {6, 0},
            {26, 4, 2024}
    };

    ticket[0] = ticket1;
    ticket[1] = ticket2;

    strcpy(train[0].trainId, "T1");
    strcpy(train[0].departure.stationId, "S1");
    strcpy(train[0].departure.laneId, "L1");
    strcpy(train[0].departure.stationName, "Huat Ah");
    strcpy(train[0].destination.stationId, "S2");
    strcpy(train[0].destination.laneId, "L1");
    strcpy(train[0].destination.stationName, "Gokfa");
    train[0].date = ticket1.date;
    train[0].departureTime = ticket1.departureTime;

    initSeats(&train[0]);
    train[0].seats[0][0][0] = 'X';
    train[0].seats[0][0][1] = 'X';
    train[0].seatCount = 2;

    fwrite(&numberOfTrain, sizeof(int), 1, ticketBookingFile);
    fwrite(&numberOfTicket, sizeof(int), 1, ticketBookingFile);
    fwrite(train, sizeof(Train), numberOfTrain, ticketBookingFile);
    fwrite(ticket, sizeof(Ticket), numberOfTicket, ticketBookingFile);

    printf("Printed successfully\n");
    fclose(ticketBookingFile);

    free(train);
    free(ticket);
}

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    mainMenu();
    endwin();
    return 0;
}
