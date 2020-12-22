#define S0 0 // initial "get ready to play state" - press play
#define S1 1 // Game state (youre actually playing) - 60 seconds
#define S2 2 // Final State (tells you the score) - 2 seconds

void StateMachine()
{
    int state = S0;
    while (1)
    {
        switch (state)
        {
        S0:
            if (playbutton is clicked)
            {
                state = S1;
            }
            else
            {
                // do nothing
            }
            break;
        S1:
            // provide moles to whack
            // display score
            // display 60 second timer
            // wait for 60 seconds

            state = S2;
            break;
        S2:
            // display GAME OVER
            // display score
            // wait for 3 seconds
            state = S0;
            break;
        }
    }
}