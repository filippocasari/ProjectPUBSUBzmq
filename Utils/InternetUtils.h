//
// Created by Filippo Casari on 02/10/21.
//

#ifndef PROJECTPUBSUBZMQ_INTERNETUTILS_H
#define PROJECTPUBSUBZMQ_INTERNETUTILS_H
char* getIp(){
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* display result */
    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}
#endif //PROJECTPUBSUBZMQ_INTERNETUTILS_H
