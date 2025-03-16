# Scorciatoie da Tastiera

[English version](SHORTCUTS.md)

## Finestra Layout UI

`Ctrl + L` per aprire Finestra Layout UI.

## Barra di stato nelle schermate

Quando il focus della tastiera è su una schermata di un Circuito o Pannello/Banco,
puoi usare `Shift + Z` per mostrare/nascondere la barra di stato.

In questo modo puoi risparmiare spazio dello schermo quando molte schermate sono aperte allo stesso tempo.

## Renderizza Schermata su SVG

Premi `Ctrl+R` su una schermata attiva, una finestra di dialogo apparirà per selezionare il file SVG di destinazione.

## Zoom schermata

Puoi ingrandire/rimpicciolire i contenuti usand `Ctrl` e ruotando la rotella del mouse.
Oppure puoi inserire il livello di zoom nella barra di stato della schermata.

## Pan schermata

Puoi scorrere la visuale premendo `Alt + Click Destro` e muovendo il puntatore in giro.

## Finestra Proprietà

### Campi oggetto

Per rimuovere un oggetto da un campo, elimina il testo e premi `Enter`.

### Campi numerici

I campi numerici possono essere modificati con la rotella del mouse se ci passi sopra con il cursore.
Se sono all'interno di un'area di scorrimento, devi prima cliccarli per spostare il focus su di loro e poi scorrere la rotella del mouse per cambiare numero.

### Nodes Tab

Nella finestra di Proprietà dell'oggetto, la scheda "Nodi" mostra una lista di tutti i nodi correlati all'oggetto.
Facendo doppio click su un elemento, verrà aperta (o portata in primo piano se già aperta) una finestra del circuito centrata sul nodo selezionato.

Se `Shift` è premuto mentre si clicca, una nuova finestra verrà aperta a prescindere se esistono già finestre per lo stesso circuito.
Se `Alt` è premuto mentre si clicca, il livello di zoom della finestra non verrà aumentato se minore di 100%.

## Modalità di modifica

### Default

Puoi:
- Aggiungere/rimuovere nodi nei circuiti.
- Ruotare nodi (`Click Destro` orario o `Shift + Click Destro` antiorario).
- Specchiare nodi (`Ctrl + Click Destro` per contatti a deviatore, nodo semplice, alimentazione relè combinatori).
- Ruotare la posizione dell'etichetta del nodo (`Alt + Click Destro`).
- Aggiungere/Rimuovere cavi.

Puoi spostare un solo nodo alla volta trascinandolo. Non puoi spostare i cavi.
Per piazzare un nodo sopra un cavo esistente, tieni premuto `Shift` mentre rilasci il mouse. Il cavo verrà diviso in 2 segmenti.

Doppio click su un nodo/cabo per mostrare la finestra di modifica.

Questa è la modalità di default.
Se ti trovi in un'altra modalità, premi `Esc` per tornare alla modalità di Default.

### Modifica Cavi

Quando sei in modalità modifica cavi, puoi creare il percorso di un cavo con:
- `Click Sinistro` sul bordo di una cella per cominciare un cavo. Apparirà una piccola linea verde.
- `Click Sinistro` su altre celle per far andare il cavo in orizzontale o verticale dal suo ultimo punto.
- `Click Destro` sul bordo di una cella per terminare il cavo. Il cavo diventerà rosso.
- Premere `Enter` per confermare o `Esc` per annullare.

Usa `Ctrl + Z` durante la modifica per annullare l'ultimo segmento del cavo.

In modalità Default puoi cominciare la Modifica Cavi con:
- `Shift + Click Sinistro` su un cavo esistente.
- `Doppio Click` su un cavo esistente, successivamente `Modifica Percorso` nella finestra di dialogo.
- Premi `Cavo` nella barra degli strumenti per creare un nuovo cavo.
- Premi la scorciatoia `C` per creare un nuovo cavo.

### Modalità selezione

In questa modalità puoi selezionare più nodi e cavi per eliminarli, spostarli o copiarli e incollarli.
Non puoi ruotare i nodi o modificare il percorso dei cavi.

Dalla modalità Default, puoi entrare in Selezione premendo `Shift + S` in una schermata di Circuiti.

Puoi uscire dalla modalità Selezione premendo `Esc`.

Per selezionare gli elementi puoi fare:
- `Click Sinistro` su nodo/cavo per selezionarli, la selezione precedente sarà annullata.
- Trascinare con il mouse da un'area vuota, tutti gli elementi toccati dalla banda saranno selezionati. La selezione precedente sarà annullata.
- `Ctrl + Click Sinistro` su un nodo/cavo per aggiungerlo/rimuoverlo dalla selezione attuale.
- `Click Sinistro` su un'area vuota per annullare la selezione attuale.
- `Ctrl + A` per selezionare tutto nella schermata attiva.
- `Shift + Ctrl + A` per invertire la selezione.

Per copiare la selezione attuale, premi `Ctrl + C`.
Per incollare nella schermata attiva, premi `Ctrl + V`.
Per sostituire oggetti o modificare in gruppo nella selezione attuale, premi `Ctrl + D` o `Shift + Ctrl + D`.

# Scorciatoie Nodi

Puoi vederlo scorrendo il mouse sopra un elemento della barra degli strumenti.
Il nome del nodo e la sua scorciatoia saranno mostrati nel tooltip.

## Base
- `P` per sorgente alimentazione
- `F` per nodo semplice
- `C` per un nuovo cavo

## Contatti
- `B` per contatti Pulsante
- `K` per contatti Leva
- `R` per contatti Relè
- `X` per contatti Relè Schermo A/B

## Nodi di alimentazione
- `E` per alimentazione Relè
- `L` per alimentazione Lampadina
- `M` per alimentazione Elettromagnete
- `V` per alimentazione Relè Schermo
- `U` per nodo suoneria

## Varie
- `D` per nodo diodo
- `I` per nodo inversione polarità
- `O` per interruttore On/Off
- `Q` per nodo bifilarizzatore

## Circuito remoto
- `T` per il nodo di connessione a circuito remoto

