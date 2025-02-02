#!/bin/bash

# Parser bauen
gcc -o Parser Parser.c Stack.c Namensliste.c Codegenerierung.c Lexer.c -Wall > /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo "Fehler beim Bauen des Parsers. Skript wird beendet."
  exit 1
fi

# Programme im Ordner pl0 ausführen
processed_programs=()

while IFS=: read -r prog_num inputs expected_output; do
  # Überspringe doppelte Programmnummern
  if [[ " ${processed_programs[@]} " =~ " $prog_num " ]]; then
    continue
  fi
  processed_programs+=("$prog_num")

  file="pl0/min$prog_num.pl0"
  
  if [ ! -f "$file" ]; then
    echo "Datei $file nicht gefunden."
    continue
  fi
  
  # Parser ausführen
  ./Parser "$file" > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo "Fehler beim Parsen von $file."
    continue
  fi
  
  # Prüfen, ob output.cl0 erzeugt wurde
  if [ ! -f "output.cl0" ]; then
    echo "output.cl0 wurde nach dem Parsen von $file nicht erzeugt."
    continue
  fi
  
  # Eingaben an das Programm weitergeben
  if [ -n "$inputs" ]; then
    actual_output=$(echo "$inputs" | ./r2416 output.cl0)
  else
    actual_output=$(./r2416 output.cl0)
  fi

  # Konvertiere die tatsächliche Ausgabe für RegEx-Vergleich
  normalized_output=$(echo "$actual_output" | sed ':a;N;$!ba;s/\n/\\n/g')

  # Validierung der Ausgabe
  if [ -n "$expected_output" ]; then
    if [[ "$expected_output" == ^* ]]; then
      # RegEx-Prüfung
      if ! [[ "$normalized_output" =~ $expected_output ]]; then
        echo "WARNUNG: Ausgabe von $file weicht ab. Erwartet (RegEx): $expected_output, erhalten: $actual_output"
      fi
    else
      # Exakte Prüfung
      if [ "$actual_output" != "$expected_output" ]; then
        echo "WARNUNG: Ausgabe von $file weicht ab. Erwartet: $expected_output, erhalten: $actual_output"
      fi
    fi
  fi
done < input_list.txt

