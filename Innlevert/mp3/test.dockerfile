FROM httpd:alpine as web

COPY ./http.conf /usr/local/apache2/conf/httpd.conf
COPY ./cgi/* /usr/local/apache2/cgi-bin/

RUN apk update && apk add python3 xmlstarlet wget curl pwgen


FROM httpd:alpine as api

COPY ./api.conf /usr/local/apache2/conf/httpd.conf
COPY ./cgi/* /usr/local/apache2/cgi-bin/
COPY ./Dikt.db /usr/local/apache2/db/

RUN apk update && apk add python3 sqlite xmlstarlet wget && chmod +w /usr/local/apache2/db/Dikt.db

