FROM gcc:4.9

WORKDIR /app

COPY . ./

EXPOSE 5000

RUN gcc main.c -o udp

# ENTRYPOINT ["./udp", "0.0.0.0", "$TALK_PORT", "$SERVER_HOST", "$TALK_PORT"]

ENTRYPOINT ["tail", "-F", "/dev/null"]
