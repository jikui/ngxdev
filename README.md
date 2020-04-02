# ngxdev
Introducation:

ALG MODULE for NGINX. Currently only ftp alg is supported.

How TO Use?
1) configure for adding the alg module into the nginx
  ./configure --with-stream --with-stream_alg ...
2) add the "alg ftp" into the upstream server syntax scope.
   server {
        listen 60.60.60.77:2121;
        proxy_timeout 65534;
        proxy_pass vpnftp1;
        alg ftp;
    }

