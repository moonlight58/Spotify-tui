#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oauth.h"
#include "utils.h"

int main()
{
    // Test RequestUserAuth
    loadEnv(".env");
    RequestUserAuth();
    getProfile(getenv("ACCESS_TOKEN"));

    return 0;
}