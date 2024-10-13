FROM ubuntu

COPY server /var/server
RUN apt-get -y update && apt-get -y install build-essential
RUN cd /var/server && make clean && make all
CMD ["/var/server/server"]
