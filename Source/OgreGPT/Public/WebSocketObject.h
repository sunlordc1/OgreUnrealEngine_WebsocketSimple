#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IWebSocket.h"
#include "WebSocketObject.generated.h"

// Delegate definitions
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWebSocketConnectedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWebSocketMessageSignature, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWebSocketClosedSignature, int32, StatusCode, const FString&, Reason, bool, bWasClean);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWebSocketErrorSignature, const FString&, Error);

UCLASS(BlueprintType)
class OGREGPT_API UWebSocketObject : public UObject
{
    GENERATED_BODY()

public:
    // Tạo và kết nối WebSocket
    UFUNCTION(BlueprintCallable, Category = "Ogre|WebSocket")
    static UWebSocketObject* Connect(const FString& ServerUrl);

    // Gửi tin nhắn
    UFUNCTION(BlueprintCallable, Category = "Ogre|WebSocket")
    void SendMessage(const FString& Message);

    // Đóng kết nối (C++ internal)
    void Close();

    // Ngắt kết nối và thông báo BP
    UFUNCTION(BlueprintCallable, Category = "Ogre|WebSocket")
    void Disconnect();

    // Delegate cho BP
    UPROPERTY(BlueprintAssignable, Category = "Ogre|WebSocket")
    FWebSocketConnectedSignature OnConnected;

    UPROPERTY(BlueprintAssignable, Category = "Ogre|WebSocket")
    FWebSocketMessageSignature OnMessage;

    UPROPERTY(BlueprintAssignable, Category = "Ogre|WebSocket")
    FWebSocketClosedSignature OnClosed;

    UPROPERTY(BlueprintAssignable, Category = "Ogre|WebSocket")
    FWebSocketErrorSignature OnConnectionError;

protected:
    void Init(const FString& ServerUrl);

    void HandleConnected();
    void HandleMessage(const FString& Message);
    void HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    void HandleConnectionError(const FString& Error);
    void HandleAppPreExit();
    void HandleWorldTearDown(UWorld* World);

    virtual void BeginDestroy() override;

private:
    TSharedPtr<IWebSocket> WebSocket;
    FDelegateHandle OnPreExitHandle;
    FDelegateHandle OnWorldTearDownHandle;
};
