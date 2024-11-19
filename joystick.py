import pygame
import time

# Pygame initialisieren
pygame.init()

# Joystick initialisieren
pygame.joystick.init()

# Überprüfen, ob ein Joystick erkannt wurde
if pygame.joystick.get_count() == 0:
    print("Kein Joystick gefunden!")
else:
    # Den ersten Joystick auswählen
    joystick = pygame.joystick.Joystick(0)
    joystick.init()

    print(f"Joystick Name: {joystick.get_name()}")
    print(f"Anzahl Achsen: {joystick.get_numaxes()}")
    print(f"Anzahl Tasten: {joystick.get_numbuttons()}")

    # Listen für vorherige Werte von Achsen und Tasten
    previous_axes = [0.0] * joystick.get_numaxes()
    previous_buttons = [0] * joystick.get_numbuttons()

    # Endlosschleife, um Eingaben zu überwachen und nur geänderte Werte zu printen
    try:
        while True:
            # Alle Pygame-Events abfragen
            pygame.event.pump()

            # Achsenwerte auslesen und vergleichen
            for i in range(joystick.get_numaxes()):
                axis_value = round(joystick.get_axis(i), 3)  # auf 3 Dezimalstellen runden
                if axis_value != previous_axes[i]:  # Nur printen, wenn sich der Wert ändert
                    print(f"Achse {i}: {axis_value}")
                    previous_axes[i] = axis_value

            # Tastenwerte auslesen und vergleichen
            for i in range(joystick.get_numbuttons()):
                button_value = joystick.get_button(i)
                if button_value != previous_buttons[i]:  # Nur printen, wenn sich der Wert ändert
                    print(f"Taste {i}: {button_value}")
                    previous_buttons[i] = button_value

            # Etwas warten, um CPU zu schonen
            time.sleep(0.1)

    except KeyboardInterrupt:
        # Aufräumen bei Abbruch
        print("\nProgramm beendet.")
        pygame.quit()
