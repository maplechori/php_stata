FROM php:7.4-cli

RUN apt-get update -y && apt-get upgrade -y && apt-get install git -y
# RUN git clone https://github.com/maplechori/php_stata.git 
RUN mkdir php_stata
COPY . php_stata
RUN  cd php_stata \ 
        && phpize \ 
        && ./configure --enable-stata \
        && make -j "$(nproc)" \
        && make install \
        && docker-php-ext-enable stata

RUN mkdir tests
COPY tests tests
RUN ls tests && cd tests && php writing.php && php reading.php
