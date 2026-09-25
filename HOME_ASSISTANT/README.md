# Configurations

> ⚠️ Il existe différentes configurations en fonction du contexte d'installation.
> * Le compteur est en mode Historique (En Monophasé ou Triphasés)
> * Le compteur est en mode Standard sans injection (En Monophasé ou Triphasés)
> * Le compteur est en mode Standard avec injection (Panneaux Solaires) (En Monophasé ou Triphasés)

# Monophasé

## Compteur en mode Historique

Dans ce contexte, le chargeur doit respecter la relation IINST (A) <= ISOUSC (A) pour éviter les risques de disjonction.
Utilisez le fichier [Mode_Historique_1Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Historique_1Ph.yml).

## Compteur est en mode Standard sans injection

Dans ce contexte, le chargeur doit respecter la relation SINSTS (VA) <= PCOUP x 1000 (kVA) pour éviter les risques de disjonction.
Utilisez le fichier [Mode_Standard_1Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Standard_1Ph.yml).

## Compteur est en mode Standard avec injection (Panneaux Solaires)

> **WIP...**, je n'ai aucun environnement de test pour cette configuration, il est nécessaire de jouer avec les paramètres :
> * SINST (Puissance app. Instantanée)
> * PCOUP (Puissance app. de coupure)
> * EAIT (Energie active injectée totale)
> * Peut être autre chose ???

Avec l'ajout d'un bouton permettant de privilégier l'énergie solaire permettant de minimiser EAIT, mais qui permet une recharge
rapide respectant SINST <= PCOUP si besoin.

Utilisez le fichier [Mode_Standard_Injection_1Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Standard_Injection_1Ph.yml).

# Triphasés

## Compteur en mode Historique

Dans ce contexte, le chargeur doit respecter la relation (IINST1 ou IINST2 ou IINST3) <= ISOUSC pour éviter les risques de disjonction.
Utilisez le fichier [Mode_Historique_3Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Historique_3Ph.yml).

## Compteur est en mode Standard sans injection

Dans ce contexte, le chargeur doit respecter la relation SINST <= PCOUP pour éviter les risques de disjonction.
Utilisez le fichier [Mode_Standard_3Ph.yml](https://github.com/bastoon577-lang/ET3K_SmartCharger/blob/main/HOME_ASSISTANT/Mode_Standard_3Ph.yml).

## Compteur est en mode Standard avec injection (Panneaux Solaires)

Ce cas est soumis à différents critères:
 * Convertisseur Monophasé sur une phase uniquement (il est nécessaire de savoir où)
 * Convertisseur Triphasée (Dans ce cas, ça va)