#include "task_wifi.h"

void startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    xSemaphoreGive(xBinarySemaphoreInternet);
}

void startSTA()
{
    if (WIFI_SSID.isEmpty())
    {
        vTaskDelete(NULL);
    }

    WiFi.mode(WIFI_STA);

    if (WIFI_PASS.isEmpty())
    {
        WiFi.begin(WIFI_SSID.c_str());
    }
    else
    {
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)  // 2 second timeout
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("✅ WiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        xSemaphoreGive(xBinarySemaphoreInternet);
    }
    else
    {
        Serial.println("❌ WiFi connection failed, starting AP");
        startAP();
    }
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    startSTA();
    return false;
}

void wifi_task(void* param)
{
    // Mount LittleFS, load credentials, start AP if none saved
    check_info_File(0);

    while (true)
    {
        if (check_info_File(1))
        {
            if (!Wifi_reconnect())
            {
                Webserver_stop();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
