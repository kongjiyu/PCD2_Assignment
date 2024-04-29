#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curses.h>
#include <time.h>
#pragma warning (disable:4996)

#define MAX_DAYS_IN_MONTH 42 //6 weeks
#define MAX_COACHES 4
#define MAX_ROWS 10
#define MAX_COLS 4

typedef struct {
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

void displayLaneSchedule(WINDOW* menu, LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules);
void searchLaneSchedule(WINDOW* menu, LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules);
void addLaneSchedule(WINDOW* menu, LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules);
void updateSchedule(WINDOW* menu, LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules);

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

	char* choices[] = { "Yes", "No" };
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

void draw_time_selection(WINDOW* timeWin, Time time, int highlight) {
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

void getTime(Time* time) {
	int highlight = 0;
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	WINDOW* timeWin = newwin(yMax / 2, xMax - 2, 1, 1);

	time->hours = 0;
	time->minutes = 0;

	keypad(timeWin, TRUE);  // Enable keyboard input for the window

	int ch = ' ';
	while ((ch != '\n')) {  // Continue until Enter is pressed
		clear();
		refresh();
		draw_time_selection(timeWin, *time, highlight);
		ch = wgetch(timeWin);
		switch (ch) {
		case KEY_LEFT:
			if (highlight > 0) highlight--;
			break;
		case KEY_RIGHT:
			if (highlight < 1) highlight++;
			break;
		case KEY_UP:
		case KEY_DOWN:
			if (highlight == 0) {  // Hour field
				time->hours += (ch == KEY_UP) ? 1 : -1;
				if (time->hours > 23) time->hours = 0;
				if (time->hours < 0) time->hours = 23;
			}
			else if (highlight == 1) {  // Minute field
				time->minutes += (ch == KEY_UP) ? 1 : -1;
				if (time->minutes >= 60) time->minutes = 0;
				if (time->minutes < 0) time->minutes = 59;
			}
			break;
		default:
			break;
		}
	}
	clear();
	refresh();
	delwin(timeWin);
}

void getInput(char inputs[], char* title) {
	clear();
	refresh();
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	WINDOW* inputWin = newwin(yMax / 2, xMax / 2, yMax / 4, xMax / 4);
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

void displayTime() {
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	// Buffer to hold the formatted time
	char timeOutput[21];

	// Obtain current time
	// time_t is an arithmetic type capable of representing times
	time_t now = time(NULL);

	// Convert time_t to struct tm (local time)
	struct tm* localTime = localtime(&now);

	// Format time into the specified format: "20 Apr 2024 7:00 AM"
	// %d - Day of the month (01 to 31)
	// %b - Abbreviated month name
	// %Y - Year with century
	// %I - Hour (12-hour clock) as a zero-padded decimal number (01 to 12)
	// %M - Minute as a zero-padded decimal number (00 to 59)
	// %p - AM or PM designation
	strftime(timeOutput, sizeof(timeOutput), "%d %b %Y %I:%M %p", localTime);

	// Print the formatted time
	mvprintw(1, xMax - 22, "%s\n", timeOutput);
	refresh();
}

int displayMenu(WINDOW* window, char* choices[], int numOfChoices) {
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

		if (startChoice > 0) {
			mvwprintw(window, 1, 5, "^");
		}
		if (startChoice < numOfChoices - visibleChoices) {
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

void readStationInformation(FILE* trainScheduleFile, StationInfo* stationInfo, int* numberOfStation) {
	//variable initialization
	*numberOfStation = 0;
	char buffer[1024];

	//find station information
	while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
		if (strstr(buffer, "//Station Information")) {
			fgets(buffer, sizeof(buffer), trainScheduleFile); // Skip header
			break;
		}
	}

	//read station information
	while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
		if (buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '/' || strlen(buffer) <= 1) {
			break;
		}
		sscanf(buffer, "%[^,], %[^,], \"%[^\"]\"", stationInfo[*numberOfStation].stationId, stationInfo[*numberOfStation].laneId, stationInfo[*numberOfStation].stationName);
		(*numberOfStation)++;
	}

}

void readLaneInformation(FILE* trainScheduleFile, LaneInfo* laneInfo, int* numberOfLane) {
	//varialbe initialization
	*numberOfLane = 0;
	char buffer[1024];

	//find lane information
	while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
		if (strstr(buffer, "//Lane Information")) {
			fgets(buffer, sizeof(buffer), trainScheduleFile); // Skip header
			break;
		}
	}


	//read lane information
	while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
		if (buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '/' || strlen(buffer) <= 1) {
			break;
		}
		sscanf(buffer, "%[^,], \"%[^\"]\", %[^\n]",
			laneInfo[*numberOfLane].laneId,
			laneInfo[*numberOfLane].laneName,
			laneInfo[*numberOfLane].terminalStationId);
		(*numberOfLane)++;
	}

}

void readPriceAndDuration(FILE* trainScheduleFile, PriceDuration* priceAndDuration, int* numberOfPrice) {
	//variable initialization
	*numberOfPrice = 0;
	char buffer[1024];

	//find price and duration information
	while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
		if (strstr(buffer, "//Between station")) {
			fgets(buffer, sizeof(buffer), trainScheduleFile); //remove header
			break;
		}
	}

	//read price and duration information
	while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
		if (buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '/' || strlen(buffer) <= 1) {
			break;
		}
		sscanf(buffer, "%[^,], %[^,], %d, %d",
			priceAndDuration[*numberOfPrice].stationId1,
			priceAndDuration[*numberOfPrice].stationId2,
			&priceAndDuration[*numberOfPrice].price,
			&priceAndDuration[*numberOfPrice].duration
		);
		(*numberOfPrice)++;
	}
}

void readLaneSchedule(FILE* trainScheduleFile, LaneSchedule* laneSchedule, int* numberOfSchedule) {
	*numberOfSchedule = 0;
	char buffer[1024];
	int startReading = 0;

	while (fgets(buffer, sizeof(buffer), trainScheduleFile) != NULL) {
		if (strstr(buffer, "//Lane Schedules")) {
			startReading = 1;
			continue;
		}
		if (startReading && buffer[0] == '/' && buffer[1] == '/') {
			break;
		}
		if (startReading && sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%[^,],%d,%d:%d",
			laneSchedule[*numberOfSchedule].scheduleId,
			laneSchedule[*numberOfSchedule].laneId,
			laneSchedule[*numberOfSchedule].trainId,
			laneSchedule[*numberOfSchedule].departureStationId,
			laneSchedule[*numberOfSchedule].destinationStationId,
			&laneSchedule[*numberOfSchedule].departureDay,
			&laneSchedule[*numberOfSchedule].time.hours,
			&laneSchedule[*numberOfSchedule].time.minutes) == 8) {
			(*numberOfSchedule)++;
		}
	}
}

void readFile(LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedule) {
	FILE* trainScheduleFile;
	trainScheduleFile = fopen("trainSchedule.txt", "r");
	if (trainScheduleFile == NULL) {
		printf("Error: unable to open file\n");
		exit(-1);
	}


	readLaneInformation(trainScheduleFile, laneInfo, numberOfLane);
	readStationInformation(trainScheduleFile, stationInfo, numberOfStation);
	readPriceAndDuration(trainScheduleFile, priceAndDuration, numberOfPrice);
	readLaneSchedule(trainScheduleFile, laneSchedules, numberOfSchedule);
	fclose(trainScheduleFile);
}

void writeFile(LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedule) {
	FILE* trainScheduleFile;
	trainScheduleFile = fopen("trainSchedule.txt", "w");
	if (trainScheduleFile == NULL) {
		printf("Error: unable to open file\n");
		exit(-1);
	}

	fprintf(trainScheduleFile, "//Lane Information\n");
	fprintf(trainScheduleFile, "LaneID, Lane Name, Terminal Stations\n");
	for (int i = 0; i < *numberOfLane; i++) {
		fprintf(trainScheduleFile, "%s, \"%s\", %s\n", laneInfo[i].laneId, laneInfo[i].laneName, laneInfo[i].terminalStationId);
	}
	fprintf(trainScheduleFile, "\n\n");

	fprintf(trainScheduleFile, "//Station Information\n");
	fprintf(trainScheduleFile, "StationID, LaneID, Station Name\n");
	for (int i = 0; i < *numberOfStation; i++) {
		fprintf(trainScheduleFile, "%s, %s, \"%s\"\n", stationInfo[i].stationId, stationInfo[i].laneId, stationInfo[i].stationName);
	}
	fprintf(trainScheduleFile, "\n\n");

	fprintf(trainScheduleFile, "//Between station\n");
	fprintf(trainScheduleFile, "FromStationID, ToStationID, Price, Duration\n");
	for (int i = 0; i < *numberOfPrice; i++) {
		fprintf(trainScheduleFile, "%s, %s, %d, %d\n", priceAndDuration[i].stationId1, priceAndDuration[i].stationId2, priceAndDuration[i].price, priceAndDuration[i].duration);
	}
	fprintf(trainScheduleFile, "\n\n");

	fprintf(trainScheduleFile, "//Lane Schedules\n");
	fprintf(trainScheduleFile, "ScheduleID, LaneID, TrainID, Departure StationId, Arrival StationId, DepartureDay, DepartureTime\n");
	for (int i = 0; i < *numberOfSchedule; i++) {
		fprintf(trainScheduleFile, "%s,%s,%s,%s,%s,%d,%02d:%02d\n",
			laneSchedules[i].scheduleId, laneSchedules[i].laneId, laneSchedules[i].trainId,
			laneSchedules[i].departureStationId, laneSchedules[i].destinationStationId,
			laneSchedules[i].departureDay, laneSchedules[i].time.hours,
			laneSchedules[i].time.minutes);
	}
	fclose(trainScheduleFile);
}

void displayLaneSchedule(WINDOW* menu, LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules) {
	clear();
	refresh();

	if (*numberOfSchedules == 0) {
		printw("No schedules available.\nPress any key to exit.");
		refresh();
		getch();
		endwin();
		return;
	}

	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	WINDOW* displayWin = newwin(yMax - 2, xMax - 2, 1, 1);
	keypad(displayWin, TRUE);  // Enable keyboard input

	int linesPerPage = yMax - 20;  // Adjust line count based on window size
	int currentPage = 0;
	int totalPages = (*numberOfSchedules / linesPerPage) + (*numberOfSchedules % linesPerPage != 0);

	int ch;
	do {
		werase(displayWin);
		box(displayWin, 0, 0);
		mvwprintw(displayWin, 1, 1, "Lane Schedules Page %d of %d", currentPage + 1, totalPages);
		// Display headers
		mvwprintw(displayWin, 3, 1, "ID    |Lane   |Train   |Departure Station    |Destination Station |Departure Day  |Time");
		mvwprintw(displayWin, 4, 1, "----------------------------------------------------------------------------------------------");


		int start = currentPage * linesPerPage;
		int end = start + linesPerPage < *numberOfSchedules ? start + linesPerPage : *numberOfSchedules;
		for (int i = start; i < end; i++) {
			mvwprintw(displayWin, 6 + (i - start) * 2, 1, "%-6s| %-6s| %-7s| %-20s| %-20s| %-13d| %02d:%02d",
				laneSchedules[i].scheduleId, laneSchedules[i].laneId, laneSchedules[i].trainId,
				laneSchedules[i].departureStationId, laneSchedules[i].destinationStationId,
				laneSchedules[i].departureDay, laneSchedules[i].time.hours, laneSchedules[i].time.minutes);
		}

		mvwprintw(displayWin, yMax - 4, 1, "Use LEFT/RIGHT to scroll pages. Press 'q' to quit.");
		wrefresh(displayWin);

		ch = wgetch(displayWin);
		switch (ch) {
		case KEY_LEFT:
			if (currentPage > 0) currentPage--;
			break;
		case KEY_RIGHT:
			if (currentPage < totalPages - 1) currentPage++;
			break;
		case 'q':
			break;
		default:
			break;
		}
	} while (ch != 'q');

	// Cleanup ncurses
	delwin(displayWin);
	clear();
	refresh();
}


void searchLaneSchedule(WINDOW* menu, LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules) {
	//variable initialization
	int choice, searchDay = -1, found = 0;
	Time searchTime = { 0, 0 };
	char searchScheduleID[8] = { 0 };
	char searchLaneID[4] = { 0 };
	char searchTrainID[6] = { 0 };
	char* choices[] = {
		"Schedule ID",
		"Lane ID",
		"Train ID",
		"Day",
		"Time",
		"Exit"
	};
	int numOfChoice = sizeof(choices) / sizeof(char*);
	char* days[] = { "5", "7" };

	//open window
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	WINDOW* displayWin = newwin(yMax - 2, xMax - 2, 1, 1);
	keypad(displayWin, TRUE); // Enable keyboard input
	int linesPerPage = yMax - 20;  // Adjust line count based on window size
	int currentPage = 0;

	choice = displayMenu(menu, choices, numOfChoice);

	switch (choice) {
	case 0:
		getInput(searchScheduleID, "Enter the Schedule ID to search for (exp : SS001)");
		for (int i = 0; i < strlen(searchScheduleID); i++) {
			searchScheduleID[i] = toupper(searchScheduleID[i]);
		}
		break;
	case 1:
		getInput(searchLaneID, "Enter the Lane ID to search for");
		for (int i = 0; i < strlen(searchLaneID); i++) {
			searchLaneID[i] = toupper(searchLaneID[i]);
		}
		break;
	case 2:
		getInput(searchTrainID, "Enter the Train ID to search for");
		for (int i = 0; i < strlen(searchTrainID); i++) {
			searchTrainID[i] = toupper(searchTrainID[i]);
		}
		break;
	case 3:
		searchDay = displayMenu(menu, days, 2) == 0 ? 5 : 7;
		clear();
		refresh();
		break;
	case 4:
		getTime(&searchTime);
		break;
	default:
		break;
	}

	LaneSchedule* filteredResult = calloc(*numberOfSchedules, sizeof(LaneSchedule));
	int filteredCount = 0;
	for (int i = 0; i < *numberOfSchedules; i++) {
		if ((strlen(searchScheduleID) == 0 || strcmp(laneSchedules[i].scheduleId, searchScheduleID) == 0) &&
			(strlen(searchLaneID) == 0 || strcmp(laneSchedules[i].laneId, searchLaneID) == 0) &&
			(strlen(searchTrainID) == 0 || strcmp(laneSchedules[i].trainId, searchTrainID) == 0) &&
			(searchDay == -1 || laneSchedules[i].departureDay == searchDay) &&
			((searchTime.hours == 0 && searchTime.minutes == 0) || (laneSchedules[i].time.hours == searchTime.hours && laneSchedules[i].time.minutes == searchTime.minutes))) {
			filteredResult[filteredCount] = laneSchedules[i];
			filteredCount++;
		}
	}


	if (filteredCount == 0) {
		printw("No schedules available.\nPress any key to exit.");
		refresh();
		getch();
		clear();
		refresh();
		return;
	}
	else {
		int totalPages = (filteredCount / linesPerPage) + (filteredCount % linesPerPage != 0);
		int ch;
		do {
			werase(displayWin);
			box(displayWin, 0, 0);
			mvwprintw(displayWin, 1, 1, "Lane Schedules Page %d of %d", currentPage + 1, totalPages);
			// Display headers
			mvwprintw(displayWin, 3, 1, "ID    |Lane   |Train   |Departure Station    |Destination Station |Departure Day  |Time");
			mvwprintw(displayWin, 4, 1, "----------------------------------------------------------------------------------------------");


			int start = currentPage * linesPerPage;
			int end = start + linesPerPage < filteredCount ? start + linesPerPage : filteredCount;
			for (int i = start; i < end; i++) {
				mvwprintw(displayWin, 6 + (i - start) * 2, 1, "%-6s| %-6s| %-7s| %-20s| %-20s| %-13d| %02d:%02d",
					filteredResult[i].scheduleId, filteredResult[i].laneId, filteredResult[i].trainId,
					filteredResult[i].departureStationId, filteredResult[i].destinationStationId,
					filteredResult[i].departureDay, filteredResult[i].time.hours, filteredResult[i].time.minutes);
			}

			mvwprintw(displayWin, yMax - 4, 1, "Use LEFT/RIGHT to scroll pages. Press 'q' to quit.");
			wrefresh(displayWin);

			ch = wgetch(displayWin);
			switch (ch) {
			case KEY_LEFT:
				if (currentPage > 0) currentPage--;
				break;
			case KEY_RIGHT:
				if (currentPage < totalPages - 1) currentPage++;
				break;
			case 'q':
				break;
			default:
				break;
			}
		} while (ch != 'q');
	}

	delwin(displayWin);
	clear();
	refresh();
}

void addLaneSchedule(WINDOW* menu, LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules) {
	//display window
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	WINDOW* displayScheduleMenu = newwin(yMax - 5, xMax - 2, 3, 1);

	// Initialize newSchedule with default values
	LaneSchedule newSchedule = { "SS000", "", "", "", "", {0}, {0, 0} };
	int choice, numOfChoice;
	char confirm, ** choices;

	// Prompt user for new schedule details
	sprintf(newSchedule.scheduleId, "SS%03d", *numberOfSchedules + 1);


	//get lane
	choices = calloc(*numberOfLane, sizeof(char*));
	for (int i = 0; i < *numberOfLane; i++) {
		choices[i] = malloc(256 * sizeof(char));
		if (!choices[i]) {
			for (int j = 0; j < i; j++) free(choices[j]);
			free(choices);
			printw("Failed to allocate memory for lane choices.\n");
			return;
		}
		strcpy(choices[i], laneInfo[i].laneId);
	}
	clear();
	refresh();
	mvprintw(2, 1, "Please select new lane");
	choice = displayMenu(menu, choices, *numberOfLane);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}
	for (int i = 0; i < *numberOfLane; i++) free(choices[i]);
	free(choices);

	strcpy(newSchedule.laneId, laneInfo[choice].laneId);

	//get departure station
	choices = calloc(*numberOfStation, sizeof(char*));
	numOfChoice = 0;
	for (int i = 0; i < *numberOfStation; i++) {
		if (strcmp(stationInfo[i].laneId, newSchedule.laneId) == 0) {
			choices[numOfChoice] = malloc(256 * sizeof(char));
			if (!choices[i]) {
				for (int j = 0; j < i; j++) free(choices[j]);
				free(choices);
				printw("Failed to allocate memory for station choices.\n");
				return;
			}
			snprintf(choices[numOfChoice++], 256, "Station ID: %s, Station Name: %s", stationInfo[i].stationId, stationInfo[i].stationName);
		}
	}
	clear();
	refresh();
	mvprintw(2, 1, "Please select new departure station");
	choice = displayMenu(displayScheduleMenu, choices, numOfChoice);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}
	strcpy(newSchedule.departureStationId, stationInfo[choice].stationId);
	for (int i = 0; i < numOfChoice; i++) free(choices[i]);
	free(choices);

	//get destination station
	choices = calloc(*numberOfStation, sizeof(char*));
	numOfChoice = 0;
	for (int i = 0; i < *numberOfStation; i++) {
		if (strcmp(stationInfo[i].laneId, newSchedule.laneId) == 0 && strcmp(stationInfo[i].stationId, newSchedule.departureStationId) != 0) {
			choices[numOfChoice] = malloc(256 * sizeof(char));
			if (!choices[numOfChoice]) {
				for (int j = 0; j < i; j++) free(choices[j]);
				free(choices);
				printw("Failed to allocate memory for destination station.\n");
				return;
			}
			snprintf(choices[numOfChoice++], 256, "Station ID: %s, Station Name: %s", stationInfo[i].stationId, stationInfo[i].stationName);
		}
	}
	clear();
	refresh();
	mvprintw(2, 1, "Please select new destination station");
	choice = displayMenu(displayScheduleMenu, choices, numOfChoice);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}
	strcpy(newSchedule.destinationStationId, stationInfo[choice].stationId);
	for (int i = 0; i < numOfChoice; i++) free(choices[i]);
	free(choices);

	//get train
	do {
		getInput(newSchedule.trainId, "Enter the Train ID");
	} while (strcmp(newSchedule.trainId, "") == 0);
	for (int i = 0; newSchedule.trainId[i] != '\0'; i++) {
		newSchedule.trainId[i] = toupper(newSchedule.trainId[i]);
	}

	//get new departure day
	char* days[] = { "5", "7" };
	int numOfDays = sizeof(days) / sizeof(char*);
	mvprintw(2, 1, "Please select new departure day");
	choice = displayMenu(menu, days, numOfDays);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}
	newSchedule.departureDay = atoi(days[choice]);

	//get new departure time
	mvprintw(2, 1, "Please enter new departure time (HH:MM)");
	getTime(&newSchedule.time);



	// Display confirmation message
	mvprintw(yMax / 2 - 4, 1, "\tCheck data for Schedule %s\n", newSchedule.scheduleId);
	mvprintw(yMax / 2 - 3, 1, "\t=======================================================================================================\n");
	mvprintw(yMax / 2 - 2, 1, "\t| ScheduleID | LaneID | TrainID | Depart StationID | Arrive StationID | Departure Day | Departure Time|\n");
	mvprintw(yMax / 2 - 1, 1, "\t|============|========|=========|==================|==================|===============|===============|\n");
	mvprintw(yMax / 2, 1, "\t| %-10s | %-6s | %-7s | %-16s | %-16s | %-13d | %02d:%02d         |\n",
		newSchedule.scheduleId, newSchedule.laneId, newSchedule.trainId,
		newSchedule.departureStationId, newSchedule.destinationStationId,
		newSchedule.departureDay, newSchedule.time.hours,
		newSchedule.time.minutes);
	mvprintw(yMax / 2 + 1, 1, "\t=======================================================================================================\n");
	mvprintw(yMax / 2 + 2, 1, "\tPress any key to continue...");
	getch();
	confirm = getUserConfirmation("Confirm to add schedule?");
	if (confirm) {
		// Add the new schedule to the array
		laneSchedules[*numberOfSchedules] = newSchedule;
		(*numberOfSchedules)++;
		writeFile(laneInfo, stationInfo, priceAndDuration, laneSchedules, numberOfLane, numberOfStation, numberOfPrice, numberOfSchedules);

	}
	else {
		mvprintw(yMax / 2, xMax / 4, "Add schedule canceled.");
		getch();
		return;
	}
}

void updateSchedule(WINDOW* menu, LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules) {
	//variable declaration
	LaneSchedule updateSchedule;
	char** choices;
	int laneSelection, departureStationSelection, destinationStationSelection, index, numOfChoice, choice;

	//display window
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	WINDOW* displayScheduleMenu = newwin(yMax - 5, xMax - 2, 3, 1);

	choices = calloc(*numberOfSchedules + 1, sizeof(char*));
	if (!choices) {
		fprintf(stderr, "Memory allocation failed for choices array\n");
		return;
	}

	//get lane schedule
	for (int i = 0; i < *numberOfSchedules; i++) {
		choices[i] = malloc(256 * sizeof(char)); // Ensure this size is adequate
		if (!choices[i]) {
			// Free previously allocated memory
			for (int j = 0; j < i; j++) {
				free(choices[j]);
			}
			free(choices);
			fprintf(stderr, "Memory allocation failed for choices[%d]\n", i);
			return;
		}
		// Safely use snprintf to avoid buffer overrun
		snprintf(choices[i], 256, "ID:%s, Train:%s, Departure:%s, Destination:%s, Day: %d, Time:%02d:%02d",
			laneSchedules[i].scheduleId, laneSchedules[i].trainId,
			laneSchedules[i].departureStationId, laneSchedules[i].destinationStationId,
			laneSchedules[i].departureDay, laneSchedules[i].time.hours, laneSchedules[i].time.minutes);
	}
	clear();
	refresh();
	mvprintw(2, 1, "Please select schedule to modify");
	index = displayMenu(displayScheduleMenu, choices, *numberOfSchedules);
	updateSchedule = laneSchedules[index];
	if (index == -1) {
		clear();
		refresh();
		return;
	}

	//free choices memory
	for (int i = 0; i < *numberOfSchedules; i++) free(choices[i]);
	free(choices);

	//get lane
	choices = calloc(*numberOfLane, sizeof(char*));
	for (int i = 0; i < *numberOfLane; i++) {
		choices[i] = malloc(256 * sizeof(char));
		if (!choices[i]) {
			for (int j = 0; j < i; j++) free(choices[j]);
			free(choices);
			printw("Failed to allocate memory for lane choices.\n");
			return;
		}
		strcpy(choices[i], laneInfo[i].laneId);
	}
	clear();
	refresh();
	mvprintw(2, 1, "Please select new lane");
	choice = displayMenu(menu, choices, *numberOfLane);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}
	strcpy(updateSchedule.laneId, laneInfo[choice].laneId);
	for (int i = 0; i < *numberOfLane; i++) free(choices[i]);
	free(choices);

	//get departure station
	choices = calloc(*numberOfStation, sizeof(char*));
	numOfChoice = 0;
	for (int i = 0; i < *numberOfStation; i++) {
		if (strcmp(stationInfo[i].laneId, updateSchedule.laneId) == 0) {
			choices[numOfChoice] = malloc(256 * sizeof(char));
			if (!choices[i]) {
				for (int j = 0; j < i; j++) free(choices[j]);
				free(choices);
				printw("Failed to allocate memory for station choices.\n");
				return;
			}
			snprintf(choices[numOfChoice++], 256, "Station ID: %s, Station Name: %s", stationInfo[i].stationId, stationInfo[i].stationName);
		}
	}
	clear();
	refresh();
	mvprintw(2, 1, "Please select new departure station");
	choice = displayMenu(displayScheduleMenu, choices, numOfChoice);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}
	strcpy(updateSchedule.departureStationId, stationInfo[choice].stationId);
	for (int i = 0; i < numOfChoice; i++) free(choices[i]);
	free(choices);

	//get destination station
	choices = calloc(*numberOfStation, sizeof(char*));
	numOfChoice = 0;
	for (int i = 0; i < *numberOfStation; i++) {
		if (strcmp(stationInfo[i].laneId, updateSchedule.laneId) == 0 && strcmp(stationInfo[i].stationId, updateSchedule.departureStationId) != 0) {
			choices[numOfChoice] = malloc(256 * sizeof(char));
			if (!choices[numOfChoice]) {
				for (int j = 0; j < i; j++) free(choices[j]);
				free(choices);
				printw("Failed to allocate memory for destination station.\n");
				return;
			}
			snprintf(choices[numOfChoice++], 256, "Station ID: %s, Station Name: %s", stationInfo[i].stationId, stationInfo[i].stationName);
		}
	}
	clear();
	refresh();
	mvprintw(2, 1, "Please select new destination station");
	choice = displayMenu(displayScheduleMenu, choices, numOfChoice);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}
	strcpy(updateSchedule.destinationStationId, stationInfo[choice].stationId);
	for (int i = 0; i < numOfChoice; i++) free(choices[i]);
	free(choices);

	//get new train id
	do {
		getInput(updateSchedule.trainId, "Enter the Train ID");
	} while (strcmp(updateSchedule.trainId, "") == 0);
	for (int i = 0; updateSchedule.trainId[i] != '\0'; i++) {
		updateSchedule.trainId[i] = toupper(updateSchedule.trainId[i]);
	}

	//get new departure day
	char* days[] = { "5", "7" };
	int numOfDays = sizeof(days) / sizeof(char*);
	mvprintw(2, 1, "Please select new departure day");
	choice = displayMenu(menu, days, numOfDays);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}
	updateSchedule.departureDay = atoi(days[choice]);

	//get new departure time
	mvprintw(2, 1, "Please enter new departure time (HH:MM)");
	getTime(&updateSchedule.time);

	//let user to confirm
	clear();
	refresh();
	mvprintw(yMax / 2 - 4, 1, "\n\tConfirm to update Schedule ID %s?\n", updateSchedule.scheduleId);
	mvprintw(yMax / 2 - 3, 1, "\t=======================================================================================================\n");
	mvprintw(yMax / 2 - 2, 1, "\t| ScheduleID | LaneID | TrainID | Depart StationID | Arrive StationID | Departure Day | Departure Time|\n");
	mvprintw(yMax / 2 - 1, 1, "\t|============|========|=========|==================|==================|===============|===============|\n");
	mvprintw(yMax / 2, 1, "\t| %-10s | %-6s | %-7s | %-16s | %-16s | %-13d | %02d:%02d         |\n",
		updateSchedule.scheduleId, updateSchedule.laneId, updateSchedule.trainId,
		updateSchedule.departureStationId, updateSchedule.destinationStationId,
		updateSchedule.departureDay, updateSchedule.time.hours,
		updateSchedule.time.minutes);
	mvprintw(yMax / 2 + 1, 1, "\t|============|========|=========|==================|==================|===============|===============|\n");
	mvprintw(yMax / 2 + 2, 1, "Press any key to continue...");
	getch();
	int confirm = getUserConfirmation("Confirm to modify?");

	if (confirm) {
		// Copy all fields from updateSchedule to laneSchedules[index]
		strcpy(laneSchedules[index].laneId, updateSchedule.laneId);
		strcpy(laneSchedules[index].trainId, updateSchedule.trainId);
		strcpy(laneSchedules[index].departureStationId, updateSchedule.departureStationId);
		strcpy(laneSchedules[index].destinationStationId, updateSchedule.destinationStationId);
		laneSchedules[index].departureDay = updateSchedule.departureDay;
		laneSchedules[index].time = updateSchedule.time;
		writeFile(laneInfo, stationInfo, priceAndDuration, laneSchedules, numberOfLane, numberOfStation, numberOfPrice, numberOfSchedules);
	}
}

void deleteSchedule(LaneInfo* laneInfo, StationInfo* stationInfo, PriceDuration* priceAndDuration, LaneSchedule* laneSchedules, int* numberOfLane, int* numberOfStation, int* numberOfPrice, int* numberOfSchedules) {
	char confirm, ** choices;
	int choice, index;

	//display window
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	WINDOW* displayScheduleMenu = newwin(yMax - 5, xMax - 2, 3, 1);

	choices = calloc(*numberOfSchedules + 1, sizeof(char*));
	if (!choices) {
		fprintf(stderr, "Memory allocation failed for choices array\n");
		return;
	}

	//get lane schedule
	for (int i = 0; i < *numberOfSchedules; i++) {
		choices[i] = malloc(256 * sizeof(char)); // Ensure this size is adequate
		if (!choices[i]) {
			// Free previously allocated memory
			for (int j = 0; j < i; j++) {
				free(choices[j]);
			}
			free(choices);
			fprintf(stderr, "Memory allocation failed for choices[%d]\n", i);
			return;
		}
		// Safely use snprintf to avoid buffer overrun
		snprintf(choices[i], 256, "ID:%s, Train:%s, Departure:%s, Destination:%s, Day: %d, Time:%02d:%02d",
			laneSchedules[i].scheduleId, laneSchedules[i].trainId,
			laneSchedules[i].departureStationId, laneSchedules[i].destinationStationId,
			laneSchedules[i].departureDay, laneSchedules[i].time.hours, laneSchedules[i].time.minutes);
	}
	clear();
	refresh();
	mvprintw(2, 1, "Please select schedule to modify");
	choice = displayMenu(displayScheduleMenu, choices, *numberOfSchedules);
	if (choice == -1) {
		clear();
		refresh();
		return;
	}

	// Display details of the schedule to be deleted
	clear();
	refresh();
	mvprintw(yMax / 2 - 4, 1, "\tDetails for Schedule ID %s:", laneSchedules[choice].scheduleId);
	mvprintw(yMax / 2 - 3, 1, "\t=======================================================================================================\n");
	mvprintw(yMax / 2 - 2, 1, "\t| ScheduleID | LaneID | TrainID | Depart StationID | Arrive StationID | Departure Day | Departure Time|\n");
	mvprintw(yMax / 2 - 1, 1, "\t|============|========|=========|==================|==================|===============|===============|\n");
	mvprintw(yMax / 2, 1, "\t| %-10s | %-6s | %-7s | %-16s | %-16s | %-13d | %02d:%02d         |\n",
		laneSchedules[choice].scheduleId, laneSchedules[choice].laneId, laneSchedules[choice].trainId,
		laneSchedules[choice].departureStationId, laneSchedules[choice].destinationStationId,
		laneSchedules[choice].departureDay, laneSchedules[choice].time.hours,
		laneSchedules[choice].time.minutes);
	mvprintw(yMax / 2 + 1, 1, "\t|============|========|=========|==================|==================|===============|===============|\n");
	mvprintw(yMax / 2 + 2, 1, "\tPress any key to continue...");
	getch();
	confirm = getUserConfirmation("Confirm to delete?");

	if (confirm) {
		for (int i = choice; i < --(*numberOfSchedules); i++) {
			laneSchedules[i] = laneSchedules[i + 1];
		}
		writeFile(laneInfo, stationInfo, priceAndDuration, laneSchedules, numberOfLane, numberOfStation, numberOfPrice, numberOfSchedules);
		mvprintw(yMax / 2, xMax / 4, "Schedule successfully deleted.");
		getch();
	}
	else {
		mvprintw(yMax / 2, xMax / 4, "Delete schedule canceled.");
	}
	clear();
	refresh();
}

void laneScheduleMenu(WINDOW* menu) {
	LaneInfo laneInfo[4];
	StationInfo stationInfo[50];
	PriceDuration priceAndDuration[50];
	LaneSchedule laneSchedules[500];

	int numberOfLane = 0;
	int numberOfStation = 0;
	int numberOfPrice = 0;
	int numberOfSchedule = 0;

	char* choices[] = {
		"Display lane Schedule",
		"Search Lane Schedule",
		"Add Lane Schedule",
		"Update Lane Schedule",
		"Delete Lane Schedule",
		"Exit"
	};
	int numOfChoice = sizeof(choices) / sizeof(char*);
	int choice;

	// Read in the data from the files
	readFile(laneInfo, stationInfo, priceAndDuration, laneSchedules, &numberOfLane, &numberOfStation, &numberOfPrice, &numberOfSchedule);

	do {
		choice = displayMenu(menu, choices, numOfChoice);

		switch (choice) {
		case 0:
			displayLaneSchedule(menu, laneInfo, stationInfo, priceAndDuration, laneSchedules, &numberOfLane, &numberOfStation, &numberOfPrice, &numberOfSchedule);
			break;
		case 1:
			searchLaneSchedule(menu, laneInfo, stationInfo, priceAndDuration, laneSchedules, &numberOfLane, &numberOfStation, &numberOfPrice, &numberOfSchedule);
			break;
		case 2:
			addLaneSchedule(menu, laneInfo, stationInfo, priceAndDuration, laneSchedules, &numberOfLane, &numberOfStation, &numberOfPrice, &numberOfSchedule);
			break;
		case 3:
			updateSchedule(menu, laneInfo, stationInfo, priceAndDuration, laneSchedules, &numberOfLane, &numberOfStation, &numberOfPrice, &numberOfSchedule);
			break;
		case 4:
			deleteSchedule(laneInfo, stationInfo, priceAndDuration, laneSchedules, &numberOfLane, &numberOfStation, &numberOfPrice, &numberOfSchedule);
			break;
		case 5:
			choice = 999;
			break;
		default:
			break;
		}
	} while (choice != 999);
}

void mainMenu() {
	//get window maximum size
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);

	//initialize choices
	char* choices[] = {
			"Staff",
			"Member",
			"Exit"
	};
	int numOfChoices = sizeof(choices) / sizeof(char*);
	int choice;

	//create window
	WINDOW* menu = newwin(10, xMax - 2, yMax - 10, 1);
	keypad(menu, TRUE);

	//loop for menu
	do {
		choice = displayMenu(menu, choices, numOfChoices);

		//clear menu and proceed
		clear();
		refresh();

		//proceed to the next function
		switch (choice) {
		case 0:
			laneScheduleMenu(menu);
			break;
		case 1:
			break;
		case 2: //exit
			choice = 999;
			break;
		default:
			break;
		}
	} while (choice != 999);
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