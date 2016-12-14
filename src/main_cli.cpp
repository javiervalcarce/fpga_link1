// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
/**
 * Programa de manipulación de bits y registros de una FPGA con el diseÃ±o FPGALINK1.
 * Ver el directorio vhdl/
 *
 *
 *
 */
// platform
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
// c++
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
// app
#include "librest.h"
#include "fpga_link1.h"

using fpga_link1::FpgaLink1;
using librest::RestServer;
using librest::RestServerPool;
using librest::RestServerPoolConfig;
using librest::SyncQueue;
using librest::Path;
using librest::QueryString;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1*              g_com = nullptr;
RestServerPool*         g_server1 = nullptr; 
RestServerPoolConfig    g_httpd_conf1;            // Configuración del servidor web
SyncQueue<RestServer*>  g_event_queue(10);        // Cola de eventos síncrona (bloqueante)
int                     httpd_port = 8000;
std::string             port = "/dev/cu.usbserial-FTE5IS5D";
int                     speed = 9600;
bool                    background = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowUsage();
void BackgroundMode();
void ServerEventCallback(RestServer* req, void* user_data);
void HttpDispatcher(RestServer* httpd);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

      // Process command line options
      static struct option long_options[] = {
            { "background",    no_argument,       0, 'b' },
            { "httpd-port",    required_argument, 0, 'g' },
            { "port",          required_argument, 0, 'p' },
            { "speed",         required_argument, 0, 's' },
            { 0, 0, 0, 0 }
      };

      char* endp;
      int option_index = 0;
      int c;
      
      if (argc == 1) {
            printf("%s [--background] [--httpd-port=P] [--port] [--speed] [ADDRESS | ping] [DATA]\n", argv[0]);
            exit(0);
      }

      while (1) {
            c = getopt_long (argc, argv, "bp:s:", long_options, &option_index);
            if (c == -1) {
                  break;
            }
	    
            switch (c) {
            case 0:
                  // If this option set a flag, do nothing else now
                  if (long_options[option_index].flag != 0) {
                        break;
                  }

                  printf ("option %s", long_options[option_index].name);
                  if (optarg) {
                        printf (" with arg %s", optarg);
                  }

                  printf ("\n");
                  break;

            case 'g':
                  //printf ("option -g with value `%s'\n", optarg);
                  httpd_port = atoi(optarg);
                  break;
                  
            case 'p':
                  port = optarg;
                  break;

            case 's':
                  speed = strtoul(optarg, &endp, 0);
                  if (*endp != '\0') {
                        printf("Error reading serial port speed\n");
                        return 1;
                  }
                  break;

            case 'b':
                  background = true;
                  break;

            default:
                  return 1;
            }
      }



      FpgaLink1::Error e;
      g_com = new FpgaLink1(port, speed);
      if (g_com->Init() != FpgaLink1::Error::No) {
            printf("Error initializing comunication driver\n");
            return 1;
      }


      // MODO SERVICIO EN SEGUNDO PLANO, SIRVE UNA APLICACIÓN WEB EN EL PUERTO HTTPD_PORT Y SE USA DICHA APLICACIÓN WEB
      // COMO INTERFAZ GRÁFICA DE USUARIO PARA LEER Y ESCRIBIR LOS REGISTROS.
      if (background) {
            BackgroundMode();
            return 0;
      }
      

      // MODO INTERACTIVO. EJECUCIÓN ÚNICA DE UNA TRANSACCIÓN DE LECTURA O DE ESCRITURA.
      // 
      bool read = false;
      int param_count = argc - optind;
      uint32_t address;
      uint32_t data;
      std::string str_address;
      std::string str_data;
      
      if (param_count == 1) {
            read = true;
            str_address = argv[optind];
      } else if (param_count == 2) {
            read = false;
            str_address = argv[optind];
            optind++;
            str_data = argv[optind];            
      } else {
            ShowUsage();
            return 1;
      }

      if (str_address == "ping") {
            e = g_com->Ping(500);
            if (e != FpgaLink1::Error::No) {
                  printf("error, link down\n");
                  return 1;
            }

            printf("pong!\n");
            return 0;
      }
      
      
      address = strtoul(str_address.c_str(), &endp, 0);
      if (*endp != '\0') {
            printf("Error reading address\n");
            return 1;
      }
      
      data = strtoul(str_data.c_str(), &endp, 0);
      if (*endp != '\0') {
            printf("Error reading data\n");
            return 1;
      }


      if (read) {
            e = g_com->MemoryRD32(address, &data, 500);
            if (e != FpgaLink1::Error::No) {
                  printf("read error (%d)\n", (int) e);
                  return 1;
            }

            printf("\n");
            printf("REG[%x] -> 0x%x\n", address, data);
      } else {
            e = g_com->MemoryWR32(address, data, 500);
            if (e != FpgaLink1::Error::No) {
                  printf("write error (%d)\n", (int) e);
                  return 1;                  
            }

            printf("\n");
            printf("REG[%x] <- 0x%x\n", address, data);
            printf("ok\n");
      }

      
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowUsage() {
      printf("\n");
      printf("Usage:\n");
      printf("fpgalink1_cli [--port] [--speed] ADDRESS DATA\n");
      printf("\n");
      printf("\n");
      printf("EXAMPLES:\n");
      printf("\n");
      printf("    fpgalink1_cli 0x000234 0x00aabbf1\n");
      printf("    (writes in the 32-bit register at address 0x000234 the value 0x00aabbf1)\n");
      printf("\n");
      printf("    fpgalink1_cli 0x1a\n");
      printf("    (read the value of regiser 0x1a)\n");
      printf("\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundMode() {

      
      // Configuración del servidor HTTP, no es necesario ajustar todos los parámetros, todos ellos tienen un valor por
      // defecto bastante razonable.
      g_httpd_conf1.callback_func                 = ServerEventCallback;
      g_httpd_conf1.callback_data                 = NULL;
      g_httpd_conf1.port                          = httpd_port;     
      g_httpd_conf1.use_ssl                       = false;
      //g_httpd_conf1.ssl_cer_file                  = opt.cer_file;
      //g_httpd_conf1.ssl_key_file                  = opt.key_file;
      g_httpd_conf1.min_thread_count              = 10;
      g_httpd_conf1.max_thread_count              = 10;
      g_httpd_conf1.document_root                 = librest::ExecutableFileHostDirectory("fpgalink1_www");
      g_httpd_conf1.prefix_webservice             = "api1";
      g_httpd_conf1.debug_print_http_request_line = false;
      g_httpd_conf1.debug_print_http_headers      = false;
      g_httpd_conf1.enable_debugservice           = true;
      
      RestServer::Error e;

      // Instancio el servidor HTTP nº 1
      // -------------------------------------------------------------------------------------------
      g_server1 = new RestServerPool(&g_httpd_conf1);
      e = g_server1->Init();
      if (e != RestServer::ERROR_NO) {
            printf("Servidor 1: %s\n", RestServer::ErrorDescription(e));
            return;
      }

      printf("Servidor: Hay %d hilos a la escucha en el puerto %d/tcp\n", g_httpd_conf1.max_thread_count, g_httpd_conf1.port);
      printf("Servidor: El directorio DocumentRoot del servidor es %s\n", g_httpd_conf1.document_root.c_str());

      
      // DESPACHO DE LAS PETICIONES HTTP QUE LLEGUEN.
      // BUCLE DE EVENTOS DE LA APLICACiÓN...
      RestServer* server;
      while (1) {

            server = g_event_queue.Front();  // Bloqueante, espera por una petición en la cola...
            g_event_queue.Pop();             // Elimina el elemento leído de la cola

            assert((server->ClientProtocol() == RestServer::HTTP) ||
                   (server->ClientProtocol() == RestServer::HTTPS));

            // Lo procesa
            HttpDispatcher(server);
      }
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ServerEventCallback(RestServer* req, void* user_data) {

      // A partir de la versión librest-0.4.0 aquí tenemos dos opciones:
      
      // Opción 1: Envio el evento a la cola de eventos y proceso la petición HTTP desde el hilo principal (main).
      g_event_queue.Push(req);

      // Opción 2: procesado de las peticiones HTTP en el callback directamente (mucho más rápido, sobre un 30% más
      // rápido) pero cuidado: si procesamos en el callback => este código se puede llegar a ejecutar concurrentemente
      // desde N hilos.
      //
      // HttpDispatcher(req);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HttpDispatcher(RestServer* httpd) {
      std::string file;
      std::vector<std::string> fl;
      RestServer::Error rt;
      bool per;
      Path&        p = httpd->RequestPath();
      QueryString& q = httpd->RequestQuery();

      assert(httpd->IsBlocked() == true);

      file = p.Str();
      printf("Request %s from %s:%d\n", file.c_str(), httpd->ClientIP().c_str(), httpd->ClientPort());

      switch (httpd->BlockType()) {
      case RestServer::HTTP_GET_INDEX:
            // El cliente pide la página "/"
            httpd->Send301MovedPermanently("web/index.html");
            break;

      case RestServer::HTTP_GET_FILE:
            // El cliente ha pedido un fichero en disco (un documento .html, .css, .js, .png, .pdf, etc)

            // Lista de ficheros a enviar concatenados
            fl.clear();
            fl.push_back(file);
            
            rt = httpd->SendFile(fl);
            if (rt == RestServer::ERROR_FILE_NOT_FOUND) {
                  printf("Error en SendFile(%s), el fichero no existe\n", file.c_str());
                  httpd->SendError(404);
            }
            if (rt == RestServer::ERROR_FILE_IS_NOT_REGULAR) {
                  printf("Error en SendFile(%s), el fichero no es regular\n", file.c_str());
                  httpd->SendError(404);
            }
            if (rt != RestServer::ERROR_NO) {
                  printf("Error en SendFile(%s)\n", file.c_str());
                  httpd->SendError(500);
            }

            break;

      case RestServer::HTTP_GET_WEBSERVICE_API:

            // El cliente ha llamado al servicio web con el verbo GET, por ejemplo:
            // GET /api1/echo?k1=v1&k2=v2&k3=v3 HTTP/1.1
            if (p.Component(1, &per) != "reg") {
                  httpd->SendError(400);
                  break;
            }

            uint32_t addr;
            uint32_t data;
            
            if (p.Count() == 3) {
                  // read
                  addr = static_cast<uint32_t>(p.ComponentAsInt(2, &per));

                  FpgaLink1::Error e = g_com->MemoryRD32(addr, &data, 500);
                  if (e != FpgaLink1::Error::No) {
                        printf("read error (%d)\n", (int) e);
                        httpd->SendText("error");
                        break;
                  }
                  
                  printf("\n");
                  printf("REG[%x] -> 0x%x\n", addr, data);
                  
                  std::ostringstream os;
                  os << data;
                  httpd->SendText(os.str());
                  
            } else if (p.Count() == 4) {
                  // write
                  addr = static_cast<uint32_t>(p.ComponentAsInt(2, &per));
                  if (per) {
                        httpd->SendError(400);
                        break;
                  }
                  
                  data = static_cast<uint32_t>(p.ComponentAsInt(3, &per));
                  if (per) {
                        httpd->SendError(400);
                        break;
                  }
                  
                  FpgaLink1::Error e = g_com->MemoryWR32(addr, data, 500);
                  if (e != FpgaLink1::Error::No) {
                        printf("write error (%d)\n", (int) e);
                        httpd->SendText("error");
                        break;
                  }

                  printf("\n");
                  printf("REG[%x] <- 0x%x\n", addr, data);

                  httpd->SendText("ok");
                  
            } else {
                  httpd->SendError(400);
                  break;
            }
      
      default:
            httpd->SendError(400);
            break;
      }
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
