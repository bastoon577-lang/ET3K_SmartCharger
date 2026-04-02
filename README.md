# ET3K_SmartCharger

Développement d'un système de recharge VE basé sur les plateformes :
* [ESP01](https://www.captain-arduino.fr/esp8266-01s/),
* [ET3K](https://fr.aliexpress.com/item/1005003433694455.html?dp=Cj0KCQjwj47OBhCmARIsAF5wUEExlk60GLalFcOgMwSaBVl6OryrBLvnEhBZIN-G9wakJbBSW7m_WWwaArpIEALw_wcB@251641&isdl=y&aff_fsk=_on9NVyV&src=Delupe3&aff_platform=aff_feeds&aff_short_key=_on9NVyV&pdp_npi=4%40dis%21EUR%2120.96%2120.79%21%21%21%21%21%40%2112000034155484243%21afff%21%21%21&gatewayAdapt=glo2fra&cn=251641&cv=461959&af=).

*******

Table des matières
 1. [Mes motivations](#motivation)
 2. [Concept du SmartCharger](#concept)
 3. [Clone du projet](#clone)
 4. [Compile du projet](#build)
*******

<div id='motivation'/> 

## Mes motivations

J'ai souhaité développer un système de recharge intelligent pour véhicule électrique capable d'adapter la puissance de recharge en temps réel, et ainsi éviter tout risque de surcharge du réseau électrique.

<div id='concept'/> 

## Concept du SmartCharger

Veuillez consulter le [Wiki du projet](https://github.com/bastoon577-lang/ET3K_SmartCharger/wiki) pour plus d'informations concernant le concept.

Ce projet fonctionne de paire avec le projet [Module TIC](https://github.com/bastoon577-lang/Module_TIC/wiki), et permet la lecture des données issues du compteur Linky au travers du réseau LAN en utilisant le réseau Wifi.

<div id='clone'/> 

## Clone du projet

Le clonage du projet passe par les étapes suivantes:
```
git clone https://github.com/bastoon577-lang/ET3K_SmartCharger.git
cd ET3K_SmartCharger/
git submodule update --init --recursive
```

<div id='build'/> 

## Compilation du projet

Veuillez vous référer au [Wiki de compilation](https://github.com/bastoon577-lang/ET3K_SmartCharger/wiki/Compilation-du-ET3K_SmartCharger).

###### Auteur : *Sébastien DALIGAULT*. 
