# Titolo, assi e griglia
set title "Oscilloscope"
set xlabel "Time"
set ylabel "Voltage"
set mytics 2
set grid ytics mytics
unset xtics

# Comando per chiudere il grafico con hotkey
bind "q" "unset output; exit gnuplot"

# Dati da plottare (uso comando tail di linux)
DATA = "< tail -500 ./data/analog_" . ARG1 . ".txt"
LINE_TITLE = "A" . ARG1 . " pin"

# Comando per plottare i dati
plot[0:500][0:5] DATA using ($1/1023*5) with lines linewidth 1 linetype rgb "black" title LINE_TITLE

# Aggiorno il grafico con i nuovi dati in ingresso
while(1) {
    replot
    pause 0.01
}