# ET3K_SmartCharger

Développement d'un système de recharge VE basé sur les plateformes :
* [ESP01](https://www.captain-arduino.fr/esp8266-01s/),
* [ET3K](https://fr.aliexpress.com/item/1005003433694455.html?dp=Cj0KCQjwj47OBhCmARIsAF5wUEExlk60GLalFcOgMwSaBVl6OryrBLvnEhBZIN-G9wakJbBSW7m_WWwaArpIEALw_wcB@251641&isdl=y&aff_fsk=_on9NVyV&src=Delupe3&aff_platform=aff_feeds&aff_short_key=_on9NVyV&pdp_npi=4%40dis%21EUR%2120.96%2120.79%21%21%21%21%21%40%2112000034155484243%21afff%21%21%21&gatewayAdapt=glo2fra&cn=251641&cv=461959&af=).

J'ai réalisé une carte électronique **SmartCharger** à pluger sur les borniers de l'équippement ET3K.
<div align="center"><img width="973" height="626" alt="SmartCharger" src="https://github.com/user-attachments/assets/f72fbf5d-9891-49cb-9498-30be793321af" /></div>

La carte **SmartCharger** embarque :
 * Un ESP01 (cerveau du système offrant une communication sur les réseaux Wifi),
 * Un convertisseur DC/DC (adaptation en tension de sortie ET3K 5V -> 3V3),
 * Un bouton poussoir de Reset,
 * Un convertisseur RS485/UART (communication ESP01 <-> ET3K),
 * Connecteur embrochable sur les borniers de l'ET3K.

*******

Table des matières
 1. [Mes motivations](#motivation)
 2. [Concept du SmartCharger](#concept)
 3. [Clone du projet](#clone)
 4. [Reproduction hardware](#hard)
 5. [Compilation du projet](#build)
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

<div id='hard'/> 

## Reproduction hardware

J'ai mis à disposition les sources complètes (BOM, GERBER, etc.) pour la reproduction du PCB **SmartCharger** [ici](https://oshwlab.com/sebastiendaligault577/project_hyqsfkqb) que j'ai fait imprimé chez [JLCPCB](https://jlcpcb.com/fr/?from=t1_france&gad_source=1&gad_campaignid=22816912221&gclid=CjwKCAjwxb7RBhA5EiwAQ-AAdAA14H3HSPcA5iJi0b9ELBZxiE4g4ar-gzyYgUTeqq2lm2xu-K8XbBoCG00QAvD_BwE).

J'en ai évidement quelques exemplaires n'hésitez pas à rentrer en contact avec moi en MP :wink:

<div id='build'/> 

## Compilation du projet

Veuillez vous référer au [Wiki de compilation](https://github.com/bastoon577-lang/ET3K_SmartCharger/wiki/Compilation-du-ET3K_SmartCharger).

###### Auteur : *Sébastien DALIGAULT*. 
