# Configurations

> ⚠️ Il existe différentes configurations en fonction du contexte d'installation.
> * Le compteur est en mode Historique (En Monophasé ou Triphasés)
> * Le compteur est en mode Standard avec/sans injection (En Monophasé ou Triphasés)

# Monophasé

## Compteur en mode Historique

Dans ce contexte, le chargeur doit respecter la relation IINST (A) <= ISOUSC (A) pour éviter risques de disjonction.
Utilisez le fichier [Mode_Historique_1Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Historique_1Ph.yml).


## Compteur est en mode Standard

Dans ce contexte, le chargeur doit respecter la relation SINSTS (VA) <= PCOUP x 1000 (kVA) pour éviter risques de disjonction.
Utilisez le fichier [Mode_Standard_1Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Standard_1Ph.yml).
L'interface donne accès à:
| Paramètres | Remarques |
| :--- | :---: |
| Charge solaire | Permet l'utilisation au maximum de l'énergie solaire provenant de l'installation PV |
| HC Standard (HC:HP) | Active la charge uniquement pendant les Heures Creuses |
| HC Jours Bleus | Active la charge uniquement pendant les Heures Creuses en jours bleus |
| HC Jours Blancs | Active la charge uniquement pendant les Heures Creuses en jours blancs |
| HC Jours Rouges | Active la charge uniquement pendant les Heures Creuses en jours rouges |
| Heures Super Creuses | Active la charge uniquement pendant les Heures Super Creuses |
| HC Weekends | Active la charge uniquement pendant les Heures creuses weekends |
| HC Mercredis | Active la charge uniquement pendant les Heures creuses mercredis |
| Courant limite de charge | Représente le courant maximum pouvant être absorbé par le VE |
| Courant dégradé | Représente le courant en mode dégradé (perte de données du Module TIC) |
| Heure début de charge solaire | Représente l'heure ou la borne passera dans l'état charge en mode solaire |
| Heure fin de charge solaire | Représente l'heure ou la borne passera dans l'état bloquée en mode solaire |

# Triphasés

## Compteur en mode Historique

Dans ce contexte, le chargeur doit respecter la relation (IINST1 ou IINST2 ou IINST3) <= ISOUSC pour éviter les risques de disjonction.
Utilisez le fichier [Mode_Historique_3Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Historique_3Ph.yml).

## Compteur est en mode Standard

Dans ce contexte, le chargeur doit respecter la relation SINST <= PCOUP pour éviter les risques de disjonction.
Utilisez le fichier [Mode_Standard_3Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Standard_3Ph.yml).
L'interface donne accès à:
| Paramètres | Remarques |
| :--- | :---: |
| Charge solaire | Permet l'utilisation au maximum de l'énergie solaire provenant de l'installation PV |
| HC Standard (HC:HP) | Active la charge uniquement pendant les Heures Creuses |
| HC Jours Bleus | Active la charge uniquement pendant les Heures Creuses en jours bleus |
| HC Jours Blancs | Active la charge uniquement pendant les Heures Creuses en jours blancs |
| HC Jours Rouges | Active la charge uniquement pendant les Heures Creuses en jours rouges |
| Heures Super Creuses | Active la charge uniquement pendant les Heures Super Creuses |
| HC Weekends | Active la charge uniquement pendant les Heures creuses weekends |
| HC Mercredis | Active la charge uniquement pendant les Heures creuses mercredis |
| Courant limite de charge | Représente le courant maximum pouvant être absorbé par le VE |
| Courant dégradé | Représente le courant en mode dégradé (perte de données du Module TIC) |
| Heure début de charge solaire | Représente l'heure ou la borne passera dans l'état charge en mode solaire |
| Heure fin de charge solaire | Représente l'heure ou la borne passera dans l'état bloquée en mode solaire |

#### Auteur : *Sébastien DALIGAULT*. 
