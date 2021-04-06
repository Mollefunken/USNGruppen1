FROM httpd:alpine

COPY ./http.conf /usr/local/apache2/conf/httpd.conf
COPY ./cgi/* /usr/local/apache2/cgi-bin/

RUN apk update && \
    apk add python3
