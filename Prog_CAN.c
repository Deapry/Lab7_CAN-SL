#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int main(int argc, char **argv)
{
    int fdSocketCAN, i; 
	int nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;

    struct can_filter rfilter[3]; // filtres pour 2 ID

  



    unsigned char ucCharRecu;
    unsigned char ucBufferTransmit[8];
    unsigned char ucCANtoTransmit[8];
    unsigned char ucCompteChar = 0;
    unsigned char ucCANflag;

    /*
	La première étape est de créer un socket. 
	Cette fonction accepte trois paramètres : 
		domaine/famille de protocoles (PF_CAN), 
		type de socket (raw ou datagram) et 
		protocole de socket. 
	la fonction retourne un descripteur de fichier.
	*/
	if ((fdSocketCAN = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) { // Création du socket CAN, de type RAW
		perror("Socket");
		return -1;
	}

	/*
	Ensuite, récupérer l'index de l'interface pour le nom de l'interface (can0, can1, vcan0, etc.) 
	que nous souhaitons utiliser. Envoyer un appel de contrôle d'entrée/sortie et 
	passer une structure ifreq contenant le nom de l'interface 
	*/
	if(argc == 2) // si un argument est passé au programme, on l'assigne au nom da l'interface CAN à utiliser
		strcpy(ifr.ifr_name, argv[1]);
	else strcpy(ifr.ifr_name, "can0" ); // par défaut l'interface can0

	ioctl(fdSocketCAN, SIOCGIFINDEX, &ifr);
	/*	Alternativement, zéro comme index d'interface, permet de récupérer les paquets de toutes les interfaces CAN.
	Avec l'index de l'interface, maintenant lier le socket à l'interface CAN
	*/

	/*
	
	*/
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(fdSocketCAN, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind");
		return -1;
	}


    while(1)
    {
    printf("Appuyer sur R pour lecture, W pour écriture\r\n");
    ucCharRecu = getchar();
    ucCANflag = 0;
        if(ucCharRecu == 'R')
        {

            while(ucCANflag == 0)
            {
            rfilter[0].can_id   = 0x543; 
	        rfilter[0].can_mask = 0xFFF;
	        rfilter[1].can_id   = 0x864;
	        rfilter[1].can_mask = 0xFFF;
            rfilter[2].can_id   = 0x010;
	        rfilter[2].can_mask = 0x0F0;

	        setsockopt(fdSocketCAN, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
            
            	// appel système read(). Cela bloquera jusqu'à ce qu'une trame soit disponible
	        nbytes = read(fdSocketCAN, &frame, sizeof(struct can_frame));

 	        if (nbytes < 0) 
            {
		    perror("Read");
		    return -1;
	        }

	        printf("0x%03X [%d] ",frame.can_id, frame.can_dlc);
	        for (i = 0; i < frame.can_dlc; i++)
		    printf("%02X ",frame.data[i]);
	        printf("\r\n");

	        if (close(fdSocketCAN) < 0)
            {
		    perror("Close");
		    return -1;
	        }
            printf("Continuer lecture ? Q pour quitter");
            ucCharRecu = getchar();
            if(ucCharRecu =='Q')
            {
                ucCANflag = 1;
            }
          }
        }
    else if(ucCharRecu == 'W')
        {   

            while(ucCANflag == 0)
            {
            frame.can_id = 0x010;  	// identifiant CAN, exemple: 247 = 0x0F7
	        frame.can_dlc = 7;		// nombre d'octets de données
            
            while(ucCompteChar < frame.can_dlc + 1)
            {
            ucBufferTransmit[ucCompteChar] = getchar();
            ucCompteChar++;
            }

            for(i =0; i < 7;i++)
            {
            ucCANtoTransmit[i] = ucBufferTransmit[i+1];
            }
            
	        sprintf(frame.data,ucCANtoTransmit);// données 

	        if (write(fdSocketCAN, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) 
            {
		    perror("Write");
		    return -1;
	        }

	        if (close(fdSocketCAN) < 0) 
            {
		    perror("Close");
		    return -1;
	        }     
            if(ucCharRecu =='Q')
            {
                ucCANflag = 1;
            }
          }
        }
    }
}
