#dspr-contentserver

# Install uWebSockets-0.13.0 and dependencies
RUN apt-get install -y libuv1-dev libssl-dev
RUN wget https://github.com/uWebSockets/uWebSockets/archive/v0.14.4.tar.gz -O uws.tar.gz && \
    tar xvfz uws.tar.gz && \
    cd uWebSockets-0.14.4 && \
    make && \
    make install && \
    cd ../../ && \
    rm -r uWebSockets-0.14.4 && \
    rm uws.tar.gz && \
    ln -s /usr/lib64/libuWS.so /usr/lib/libuWS.so
