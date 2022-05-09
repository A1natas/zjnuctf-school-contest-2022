FROM php:5.6-apache

COPY files /tmp/files/

COPY src /var/www/html/

RUN chown -R root:root /var/www/html/ && \
    chmod -R 755 /var/www/html && \
    mv /tmp/files/start.sh / && \
    chmod +x /start.sh && \
    sed -i 's/deb.debian.org/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    sed -i '/security/d' /etc/apt/sources.list && \
    rm -rf /tmp/files && \
    rm -rf /var/lib/apt/lists/*
    
CMD /start.sh

EXPOSE 80
