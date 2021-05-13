// Some arrays to hold player specific pins
// Player array values: X, Y, LED, Button
const int player[2][4] = {
    {A1, A2, 34, 22},
    {A4, A5, 35, 23}
};
int currentCoordinates_X = 1;
int currentCoordinates_Y = 1;

// Keeps count of how many games have been played, for the purpose of alternating which player goes first.
int games = 0;

// The actual gameboard, keeping track of which squares have been taken by which player, if either.
// 0 = untaken, 1 = red, 2 = blue (In hindsight, -1 should have been untaken, then 0 could have been Red as it otherwise is, and likewise for blue.
byte board[3][3];

// This array holds the pin port numbers for the RGB LEDs.
// Coordinate[y][x][0] holds the red color pin port, Coordinate[y][x][1] holds the blue color pin port.
const byte boardPins[3][3][2] = {
    { // y=0, x=0-2
        // Z == 0 = Red, Z == 1 = Blue
        {36, 37},
        {38, 39},
        {40, 41}
    },
    { // x=1, y=0-2
        {46, 47},
        {42, 43},
        {44, 45}
    },
    { // x=2
        {52, 53},
        {50, 51},
        {48, 49}
    }
};

void setup() {
  // These nested For loops run through the boardPins array to set all the LED pins as outputs.
  for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
          for (int z = 0; z < 2; z++) {
              pinMode(boardPins[x][y][z], OUTPUT);
          }
      }
  }

  // Red LED used for indicating it's Red's turn.
  pinMode(34, OUTPUT);
  // Blue LED used for indicating it's Blue's turn.
  pinMode(35, OUTPUT);
  // Player Red's Select button.
  pinMode(22, INPUT);
  // Player Blue's Select button.
  pinMode(23, INPUT);
  // Red's X/Y inputs from the knobs.
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  // Blue's X/Y inputs from the knobs.
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
}

void loop() {
    // lightsOut() turns off all boardPins.
    lightsOut();

    // Resetting of board, setting each spot to untaken (0).
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            board[y][x] = 0;
        }
    }

    // Depending on how many games have been had, turn is set. false(0) = red, true(1) = blue
    bool turn = games % 2 == 0;
    // The game will only run until gameOver is true. gameOver happens when a player wins, or there is a draw (no untaken squares left).
    bool gameOver = false;
    // The draw bool. Used to determine why gameOver was set. In hindssight, perhaps gameOver should have been a Byte.
    bool draw = false;
    
    while (!gameOver) {
        // Current player takes their turn in playerTurn()
        playerTurn(turn);
         
        // Check if current player won
        if (checkWinner(turn))
        {
            gameOver = true;
            break;
        }
        // Checks if it's a draw
        if (isADraw())
        {
            gameOver = true;
            draw = true;
            break; 
        }
        
        // Flips whose turn it is.
        turn = turn ? false : true;
    }
    endOfGame(turn, draw);
}

// Runs the board array against the boardPins array, turns and off the pins as appropriate.
void display() {
    // 0 = untaken, 1 = red, 2 = blue
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            byte cell = board[y][x];
            if (cell == 1) {
                digitalWrite(boardPins[y][x][0], HIGH);
                digitalWrite(boardPins[y][x][1], LOW);
            }
            else if (cell == 2) {
                digitalWrite(boardPins[y][x][0], LOW);
                digitalWrite(boardPins[y][x][1], HIGH);
            }
            else {
                digitalWrite(boardPins[y][x][0], LOW);
                digitalWrite(boardPins[y][x][1], LOW);
            }
        }
    }
}
// Where the current player takes their turn, moving around on the board and making their selection.
void playerTurn(bool turn) {
    bool done = false;
    // flip is used to make the positional indicator blink periodically, so you are able to tell what your selector is ontop of.
    int flip = 0;

    // Player array values: X, Y, LED, Button
    // Here we turn on the player's turn indicator. A red LED for player Red, opposite for player Blue.
    digitalWrite(player[turn][2], HIGH);

    while (!done) {
        int sw = digitalRead(player[turn][3]);
        int x = analogRead(player[turn][0]);
        int y = analogRead(player[turn][1]);

        display();
        if (flip < 3) 
        { 
            digitalWrite(boardPins[currentCoordinates_Y][currentCoordinates_X][0], HIGH);
            digitalWrite(boardPins[currentCoordinates_Y][currentCoordinates_X][1], HIGH);
            flip++; 
        }
        else if(flip > 3) 
        { 
            flip = 0;
            
        }
        else { flip++; }
         
        // Check if the button is being pressed.
        if (sw == 1) {
            // Check if the current selection (currentCoordinates) is an untaken square.
            if (board[currentCoordinates_Y][currentCoordinates_X] == 0) {
                // Claim the sqaure as current player's square.
                board[currentCoordinates_Y][currentCoordinates_X] = turn + 1;
                done = true;
            }
        }
        else {
            // If the button isn't being pressed, then we may move.
            // We had to do a lot of changes to this part, so I kind of lost track of what is actually up/down and left/right.
            // The controllers were kinda weird.
            if (x < 200 && currentCoordinates_X < 2) {
                // Going up
                currentCoordinates_X++;
            }
            if (x > 800 && currentCoordinates_X > 0) {
                // Going down
                currentCoordinates_X--;
            }
            if (y < 200 && currentCoordinates_Y > 0) {
                // Go Right
                currentCoordinates_Y--;
            }
            if (y > 800 && currentCoordinates_Y < 2) {
                // Go Left
                currentCoordinates_Y++;
            }
        }
        // A short delay at the end, just so that one mild tap on the controller doesn't send you flying across the board.
        delay(150);
    }
    digitalWrite(player[turn][2], LOW);
}
bool checkWinner(bool turn) {
    // First we check for horizontal / vertical victories for current player (You cannot win if it's not your turn).
    for (int i = 0; i < 3; i++)
    {
        if (board[1][i] > 0 && board[1][i] - 1 == turn)
        {
            if (board[0][i] > 0 && board[2][i] > 0 && board[0][i] - 1 == turn && board[2][i] - 1 == turn)
            {
                return true;
            }
        }
        if (board[i][1] > 0 && board[i][1] - 1 == turn)
        {
            if (board[i][0] > 0 && board[i][2] > 0 && board[i][0] - 1 == turn && board[i][2] - 1 == turn)
            {
                return true;
            }
        }    
    }

    // Now checking for diagonal victories.
    if (board[1][1] > 0 && board[1][1] - 1 == turn)
    {
        if (board[0][0] > 0 
            && board[2][2] > 0 
            && board[0][0] - 1 == turn 
            && board[2][2] - 1 == turn) {
            return true;
        }
        if (board[2][0] > 0 
            && board[0][2] > 0 
            && board[2][0] - 1 == turn 
            && board[0][2] - 1 == turn) {
            return true;
        }
    }
    return false;
}
bool isADraw() {
    for (int x = 0; x < 3; x++)
    {
        for (int y = 0; y < 3; y++)
        {
            if (board[x][y] == 0) {
                return false;
            }
        }
    }
    return true;
}
void endOfGame(bool winner, bool isADraw) {
    games++;
    // A short delay so people can look at the board before the Victory (or draw) lightshow begins.
    delay(1000);
    lightsOut();
    // victory for the victor
    if (!isADraw) {
        // Time for a victory dance, starting by blinking-ly turning each LED on in the victor's color.
        // winner == false = Red, winner == true = Blue
        for (int x = 0; x < 3; x++)
        {
            for (int y = 0; y < 3; y++)
            {
                digitalWrite(boardPins[y][x][winner], HIGH);
                delay(75);
                digitalWrite(boardPins[y][x][winner], LOW);
                delay(75);
                digitalWrite(boardPins[y][x][winner], HIGH);
                delay(75);

            }
        }
        for (int i = 0; i < 8; i++)
        {
            digitalWrite(boardPins[0][1][!winner], HIGH);
            digitalWrite(boardPins[1][0][!winner], HIGH);
            digitalWrite(boardPins[1][2][!winner], HIGH);
            digitalWrite(boardPins[2][1][!winner], HIGH);

            digitalWrite(boardPins[0][0][winner], HIGH);
            digitalWrite(boardPins[0][2][winner], HIGH);
            digitalWrite(boardPins[1][1][winner], HIGH);
            digitalWrite(boardPins[2][0][winner], HIGH);
            digitalWrite(boardPins[2][2][winner], HIGH);
            delay(300);

            digitalWrite(boardPins[0][0][winner], LOW);
            digitalWrite(boardPins[0][2][winner], LOW);
            digitalWrite(boardPins[1][1][winner], LOW);
            digitalWrite(boardPins[2][0][winner], LOW);
            digitalWrite(boardPins[2][2][winner], LOW);
            delay(300);
        }
        delay(4000);
    }
    else {
        // draw :(
        for (int i = 0; i < 8; i++)
        {
            digitalWrite(boardPins[0][0][0], HIGH);
            digitalWrite(boardPins[0][1][1], HIGH);
            digitalWrite(boardPins[0][2][0], HIGH);
            digitalWrite(boardPins[1][0][1], HIGH);
            digitalWrite(boardPins[1][1][0], HIGH);
            digitalWrite(boardPins[1][1][1], HIGH);
            digitalWrite(boardPins[1][2][1], HIGH);
            digitalWrite(boardPins[2][0][0], HIGH);
            digitalWrite(boardPins[2][1][1], HIGH);
            digitalWrite(boardPins[2][2][0], HIGH);
            delay(300);
            digitalWrite(boardPins[0][0][0], LOW);
            digitalWrite(boardPins[0][1][1], LOW);
            digitalWrite(boardPins[0][2][0], LOW);
            digitalWrite(boardPins[1][0][1], LOW);
            digitalWrite(boardPins[1][1][0], LOW);
            digitalWrite(boardPins[1][1][1], LOW);
            digitalWrite(boardPins[1][2][1], LOW);
            digitalWrite(boardPins[2][0][0], LOW);
            digitalWrite(boardPins[2][1][1], LOW);
            digitalWrite(boardPins[2][2][0], LOW);
            delay(300);
        }
        delay(4000);
    }
}
// Turns off all lights on the board.
void lightsOut() {
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            digitalWrite(boardPins[y][x][0], LOW);
            digitalWrite(boardPins[y][x][1], LOW);
        }
    }
}
