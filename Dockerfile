FROM jaybuckeye2006/zephyr-env:v3.6.0

# 配置 west
RUN west config manifest.path workspace && \
    west config manifest.project-filter +canopennode

RUN echo "export IP=\$(nslookup host.docker.internal | grep 'Address:' | grep -Eo '([0-9]{1,3}\.){3}[0-9]{1,3}' | tail -n 1)" >> ~/.bashrc
