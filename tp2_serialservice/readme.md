# Solución TP2 
_Sistemas Operativos de Propósito General 17Co2022_

Trabajo práctico realizados en el contexto de la Carrera de Especialización de 
Sistemas Embebidos del Laboratorio de Sistemas Embebidos de la 
Universidad de Buenos Aires.

## Uso Rápido

Compilar con:
```bash
./compilar.sh
```

Correr con:
```bash
./serialService
```

## Caracterísicas

- Uso de pthreads para generar dos hilos. Uno para comunicación serial y otro
  para comunicación tcp.
- Uso de sockets para comunicación tcp, tanto para aceptar nuevas conexiones como
  para mantener vínculo con cliente entrante. 
- Capacidad de manejar reconexiones tcp con el cliente.
- Captura correcta de señales SIGTERM y SIGINT para término de programa. Se bloquea
  las señales para los threads serial y tcp por lo que solo entran en el thread principal.

## Notas

Para desarrollo resulta útil liberar el puerto 10000 con:

```bash
fuser -k 10000/tcp
```