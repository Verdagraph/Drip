/**
 * @file WifiManager.cpp
 * @brief Implementation of the WifiManager class for handling Wi-Fi connectivity.
 * @details Taken from https://github.com/Ariif0/ESPIDF-WiFi-Configuration
 */

#include "wifiManager.h"
#include "config.h"
#include <cstring>
#include <sys/stat.h> 
#include <fcntl.h>    
#include <unistd.h>   

/** @brief Logging tag for the WifiManager class. */
static const char* TAG = "WifiManager";

/** @brief Event bit for signaling successful Wi-Fi connection. */
#define WIFI_CONNECTED_BIT BIT0
/** @brief Event bit for signaling failed Wi-Fi connection. */
#define WIFI_FAIL_BIT      BIT1

/**
 * @brief Constructs a new WifiManager object.
 *
 * Initializes the event group and sets default values for member variables.
 */
WifiManager::WifiManager() : 
    m_server(nullptr),
    m_is_connected(false),
    m_retry_num(0)
{
    m_wifi_event_group = xEventGroupCreate();
}

/**
 * @brief Destroys the WifiManager object.
 *
 * Stops the web server and deletes the event group.
 */
WifiManager::~WifiManager() {
    stopWebServer();
    vEventGroupDelete(m_wifi_event_group);
}

/**
 * @brief Initializes the TCP/IP stack and default event loop.
 *
 * Registers event handlers for Wi-Fi and IP events.
 */
void WifiManager::initialize() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &eventHandler,
                                                        this,
                                                        nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &eventHandler,
                                                        this,
                                                        nullptr));
}

/**
 * @brief Checks if the device is connected to Wi-Fi in Station mode.
 *
 * @return true if connected, false otherwise.
 */
bool WifiManager::isConnected() const {
    return m_is_connected;
}

/**
 * @brief Retrieves the current IP address in Station mode.
 *
 * @return std::string containing the IP address, or empty if not connected.
 */
std::string WifiManager::getIpAddress() const {
    return m_current_ip;
}

/**
 * @brief Starts the Wi-Fi management process.
 *
 * Attempts to connect using stored credentials. If connection fails or no credentials
 * are available, switches to provisioning mode (AP).
 */
void WifiManager::start() {
    initialize();

    std::string saved_ssid, saved_pass;
    if (loadCredentials(saved_ssid, saved_pass) == ESP_OK) {
        ESP_LOGI(TAG, "Found credentials, connecting to '%s'", saved_ssid.c_str());
        connectToWifi(saved_ssid, saved_pass);
        
        EventBits_t bits = xEventGroupWaitBits(m_wifi_event_group,
                                               WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               pdMS_TO_TICKS(30000));

        if (bits & WIFI_CONNECTED_BIT) {
            startWebServer(false);
            return;
        } 
        ESP_LOGW(TAG, "Failed to connect with stored credentials");
    }
    
    startProvisioning();
}

/**
 * @brief Connects to a Wi-Fi Access Point with provided credentials.
 *
 * @param ssid The SSID of the Wi-Fi network.
 * @param password The password of the Wi-Fi network.
 * @return esp_err_t ESP_OK if connection attempt starts successfully, ESP_FAIL otherwise.
 */
esp_err_t WifiManager::connectToWifi(const std::string& ssid, const std::string& password) {
    if (ssid.empty()) {
        ESP_LOGE(TAG, "SSID cannot be empty");
        return ESP_ERR_INVALID_ARG;
    }

    stopWifi(); 
    
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid));
    if (ssid.length() < sizeof(wifi_config.sta.ssid)) {
        wifi_config.sta.ssid[ssid.length()] = '\0';
    }
    strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password));
    if (password.length() < sizeof(wifi_config.sta.password)) {
        wifi_config.sta.password[password.length()] = '\0';
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    return ESP_OK;
}

/**
 * @brief Starts Access Point mode for provisioning.
 *
 * Configures and starts an AP with a web server to receive new Wi-Fi credentials.
 */
void WifiManager::startProvisioning() {
    stopWifi();

    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, PROV_AP_SSID, sizeof(wifi_config.ap.ssid));
    strncpy((char*)wifi_config.ap.password, PROV_AP_PASS, sizeof(wifi_config.ap.password));
    wifi_config.ap.ssid_len = strlen(PROV_AP_SSID);
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.max_connection = PROV_AP_MAX_CONN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);
    ESP_LOGI(TAG, "AP started. SSID: '%s', Connect to http://" IPSTR, PROV_AP_SSID, IP2STR(&ip_info.ip));

    startWebServer(true);
}

/**
 * @brief Stops Wi-Fi in AP and/or STA mode.
 *
 * Stops the web server, Wi-Fi, and destroys network interfaces.
 */
void WifiManager::stopWifi() {
    stopWebServer();
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_netif_t* netif_sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif_sta) esp_netif_destroy(netif_sta);
    esp_netif_t* netif_ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (netif_ap) esp_netif_destroy(netif_ap);
}

/**
 * @brief Starts the HTTP web server.
 *
 * @param is_provisioning_mode True for AP provisioning mode, false for STA mode (reset endpoint only).
 */
void WifiManager::startWebServer(bool is_provisioning_mode) {
    if (m_server) return;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.lru_purge_enable = true;

    if (httpd_start(&m_server, &config) == ESP_OK) {
        if (is_provisioning_mode) {
            httpd_uri_t root_uri = {.uri = "/", .method = HTTP_GET, .handler = provisioningGetHandler, .user_ctx = this };
            httpd_register_uri_handler(m_server, &root_uri);
            httpd_uri_t connect_uri = {.uri = "/connect", .method = HTTP_POST, .handler = connectPostHandler, .user_ctx = this };
            httpd_register_uri_handler(m_server, &connect_uri);
        } else {
            httpd_uri_t reset_uri = {.uri = "/reset", .method = HTTP_GET, .handler = resetGetHandler, .user_ctx = this };
            httpd_register_uri_handler(m_server, &reset_uri);
        }
        httpd_uri_t favicon_uri = {.uri = "/favicon.ico", .method = HTTP_GET, .handler = faviconGetHandler, .user_ctx = this };
        httpd_register_uri_handler(m_server, &favicon_uri);
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
    }
}

/**
 * @brief Stops the HTTP web server.
 */
void WifiManager::stopWebServer() {
    if (m_server) {
        httpd_stop(m_server);
        m_server = nullptr;
    }
}

/**
 * @brief Handles Wi-Fi and IP events.
 *
 * @param arg Pointer to the WifiManager instance.
 * @param event_base Event base identifier.
 * @param event_id Event ID.
 * @param event_data Event-specific data.
 */
void WifiManager::eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    WifiManager* self = static_cast<WifiManager*>(arg);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (self->m_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            self->m_retry_num++;
        } else {
            self->m_is_connected = false;
            xEventGroupSetBits(self->m_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
        self->m_current_ip = ip_str;
        self->m_retry_num = 0;
        self->m_is_connected = true;
        xEventGroupSetBits(self->m_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief HTTP GET handler for serving the provisioning page.
 *
 * Serves the index.html file from LittleFS.
 *
 * @param req HTTP request handle.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t WifiManager::provisioningGetHandler(httpd_req_t *req) {
    const char* filepath = LFS_BASE_PATH "/index.html";
    
    struct stat st;
    if (stat(filepath, &st) != 0) {
        ESP_LOGE(TAG, "index.html not found in %s", LFS_BASE_PATH);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(TAG, "Failed to open index.html");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html");

    char buffer[256];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        if (httpd_resp_send_chunk(req, buffer, bytes_read) != ESP_OK) {
            close(fd);
            return ESP_FAIL;
        }
    }
    
    close(fd);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/**
 * @brief HTTP POST handler for receiving Wi-Fi credentials.
 *
 * Saves credentials and restarts the device to attempt connection.
 *
 * @param req HTTP request handle.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t WifiManager::connectPostHandler(httpd_req_t *req) {
    WifiManager* self = static_cast<WifiManager*>(req->user_ctx);
    
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    char ssid[33] = {0};
    char pass[65] = {0};
    
    if (httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'ssid' parameter");
        return ESP_FAIL;
    }
    httpd_query_key_value(buf, "password", pass, sizeof(pass));

    const char *resp_str = "<h1>Connecting...</h1><p>If successful, the device will connect to the network. If failed, it will remain in provisioning mode.</p>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    self->saveCredentials(ssid, pass);
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();

    return ESP_OK;
}

/**
 * @brief HTTP GET handler for resetting credentials.
 *
 * Clears stored credentials and restarts the device.
 *
 * @param req HTTP request handle.
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t WifiManager::resetGetHandler(httpd_req_t* req) {
    WifiManager* self = static_cast<WifiManager*>(req->user_ctx);
    
    const char* resp_str = "<h1>Credentials Cleared</h1><p>The device will restart and enter provisioning mode.</p>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    self->clearCredentialsAndRestart();
    
    return ESP_OK;
}

/**
 * @brief HTTP GET handler for favicon.
 *
 * Returns a 204 No Content response.
 *
 * @param req HTTP request handle.
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t WifiManager::faviconGetHandler(httpd_req_t *req) {
    httpd_resp_set_status(req, "204 No Content");
    return httpd_resp_send(req, NULL, 0);
}

/**
 * @brief Saves Wi-Fi credentials to Non-Volatile Storage (NVS).
 *
 * @param ssid The SSID to save.
 * @param password The password to save.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t WifiManager::saveCredentials(const std::string& ssid, const std::string& password) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;

    nvs_set_str(nvs_handle, NVS_KEY_WIFI_SSID, ssid.c_str());
    nvs_set_str(nvs_handle, NVS_KEY_WIFI_PASS, password.c_str());
    
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return err;
}

/**
 * @brief Loads Wi-Fi credentials from Non-Volatile Storage (NVS).
 *
 * @param ssid Output parameter for the loaded SSID.
 * @param password Output parameter for the loaded password.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t WifiManager::loadCredentials(std::string& ssid, std::string& password) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return err;

    size_t required_size;
    err = nvs_get_str(nvs_handle, NVS_KEY_WIFI_SSID, NULL, &required_size);
    if (err != ESP_OK) { nvs_close(nvs_handle); return err; }
    std::vector<char> ssid_buf(required_size);
    err = nvs_get_str(nvs_handle, NVS_KEY_WIFI_SSID, ssid_buf.data(), &required_size);
    if (err != ESP_OK) { nvs_close(nvs_handle); return err; }
    ssid = ssid_buf.data();
    
    err = nvs_get_str(nvs_handle, NVS_KEY_WIFI_PASS, NULL, &required_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        password = "";
    } else if (err == ESP_OK) {
        std::vector<char> pass_buf(required_size);
        err = nvs_get_str(nvs_handle, NVS_KEY_WIFI_PASS, pass_buf.data(), &required_size);
        if (err != ESP_OK) { nvs_close(nvs_handle); return err; }
        password = pass_buf.data();
    } else {
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

/**
 * @brief Clears stored credentials and restarts the device.
 */
void WifiManager::clearCredentialsAndRestart() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        esp_restart();
    }
    nvs_erase_key(nvs_handle, NVS_KEY_WIFI_SSID);
    nvs_erase_key(nvs_handle, NVS_KEY_WIFI_PASS);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    esp_restart();
}