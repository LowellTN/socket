# Orchestrateur de Scanners de Vulnérabilités par Lowell Buzzi

## Description

Ce projet implémente un orchestrateur permettant de contrôler à distance plusieurs scanners de vulnérabilités (Nmap, OWASP ZAP, et Nikto). L'orchestrateur peut envoyer des commandes de scan à des agents distants, collecter et centraliser les résultats, tout en garantissant la sécurité des communications via SSL/TLS.

## Fonctionnalités

1. **Orchestration des scanners** : 
   - Lancer des scans avec Nmap, OWASP ZAP et Nikto.
   - Configurer les paramètres de scan : cibles, types de tests, options spécifiques.
   
2. **Agent distant** : 
   - Exécuter des scans sur des hôtes distants.
   - Retourner les résultats de manière sécurisée.

3. **Sécurisation des échanges** : 
   - Communication chiffrée entre l'orchestrateur et les agents via SSL/TLS.
   
4. **Collecte et synthèse des résultats** : 
   - Agrégation des résultats des différents scanners pour fournir une vue synthétique des vulnérabilités détectées.

## Installation

1. Cloner le repository.
2. Générer ou obtenir les certificats SSL pour sécuriser les connexions.
3. Lancer le serveur (orchestrateur) et les agents avec leurs adresses et ports respectifs.

## Utilisation

!!! Le makefile en particulier l'orchestrateur est fait pour une configuration précise veuillez le modifier préalablement pour correspondre à vos configuration ip et port !!!

1. Lancer l'orchestrateur (serveur) et les agents distants.
2. Depuis l'orchestrateur, choisir l'outil de scan et les options à configurer. (veuillez regardez serveur_ssl si vous avez uinstallation particulière de nitko ou owasp zap)
3. Consulter les résultats des scans une fois les commandes exécutées. (ces résultats sont stockés dans le .txt mais vous pouvez l'afficher dans l'outil de l'orchestrateur)

## Technologies utilisées

- C (pour l'orchestrateur et les agents)
- OpenSSL (pour la sécurisation SSL/TLS)
- Nmap, Nikto, OWASP ZAP (outils de scan)

## Remarques

Le projet met l'accent sur la simplicité et la sécurité, en permettant de configurer et d'exécuter des scans de manière centralisée et sécurisée.

