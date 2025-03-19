FROM jaybuckeye2006/zephyr-env:v3.6.0

# 配置 west
RUN west config manifest.path workspace && \
    west config manifest.project-filter +canopennode

RUN echo "export IP=\$(nslookup host.docker.internal | grep 'Address:' | grep -Eo '([0-9]{1,3}\.){3}[0-9]{1,3}' | tail -n 1)" >> ~/.bashrc

USER root

# Install software-properties-common first
RUN apt-get update && apt-get install -y software-properties-common

# Now you can add the PPA
RUN add-apt-repository ppa:lely/ppa
RUN apt-get update

RUN apt-get install -y liblely-coapp-dev liblely-co-tools python3-dcf-tools
RUN apt-get install -y pkg-config

# These RUN commands are likely meant to be used in a build script, not in the Dockerfile
# RUN pkg-config --cflags liblely-coapp
# RUN pkg-config --libs liblely-coapp

RUN apt-get install -y can-utils
RUN apt-get install -y python3
RUN apt-get install -y vim

ADD ./lely/cli.py /usr/lib/python3/dist-packages/dcfgen
ADD ./lely/device.py /usr/lib/python3/dist-packages/dcf
ADD ./lely/hcxxd /usr/bin

USER user
