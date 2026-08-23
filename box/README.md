# Softadastra Box, premier profil

Une Box est un Host Linux dédié. Elle n'introduit aucune architecture différente.

## Besoins matériels

Minimum : CPU x86_64 2 coeurs, 4 Go RAM, SSD 64 Go, Ethernet gigabit, alimentation stable et refroidissement passif ou adapté au fonctionnement continu.

Recommandé : CPU x86_64 4 coeurs, 8 Go RAM, SSD NVMe 128 Go ou plus, Ethernet gigabit, Wi-Fi 5 ou plus, deux ports USB, alimentation externe de qualité, boîtier ventilé passif ou actif mesuré, et firmware permettant le redémarrage après retour secteur.

Le CPU, la RAM et le stockage sont des capacités du Host, pas des exigences d'un fabricant. Le premier prototype vise un mini-PC x86_64 compatible Debian ou Ubuntu LTS, avec SSD remplaçable et Ethernet ; aucun modèle n'est validé physiquement par ce dépôt.

## Image système

Utiliser une installation minimale Debian 12 ou Ubuntu 24.04 LTS x86_64 : système à jour, `systemd`, SSD local, réseau administré par la distribution. Installer ensuite le binaire construit et exécuter `box/install.sh` en root. Le script crée l'utilisateur et les répertoires Host, installe l'unité systemd, active puis démarre le Host. Il ne télécharge rien et ne dépend pas d'Internet après installation.

Le script refuse d'écraser un binaire ou une unité existants. Une mise à jour est donc une opération explicite distincte.

## Première mise sous tension

1. Vérifier dans le firmware le redémarrage après coupure secteur.
2. Installer Linux minimal sur SSD et appliquer les mises à jour initiales.
3. Copier le binaire `softadastra` sur la Box et lancer `sudo box/install.sh ./softadastra`.
4. Vérifier `systemctl status softadastra` puis `softadastra access` et `softadastra connectivity`.
5. Enregistrer un logiciel tiers et vérifier son démarrage, arrêt et restauration.

## Validation matérielle à effectuer

- Fonctionnement continu, température et consommation mesurés sur le matériel réel.
- Coupure et retour Ethernet/Wi-Fi sans perte du contrôle local.
- Coupure secteur puis retour, avec redémarrage firmware activé.
- Plusieurs reboot et vérification des registrations restaurées.
- Processus long, serveur local, arguments, écriture de fichiers et crash réels.

Le prototype physique reste non construit tant qu'une machine réelle n'a pas été assemblée, installée et testée.
