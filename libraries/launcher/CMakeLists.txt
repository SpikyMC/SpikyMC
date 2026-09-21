cmake_minimum_required(VERSION 3.15)
project(launcher Java)
find_package(Java 1.7 REQUIRED COMPONENTS Development)

include(UseJava)
set(CMAKE_JAVA_JAR_ENTRY_POINT green.spiky.mc.EntryPoint)
set(CMAKE_JAVA_COMPILE_FLAGS -target 7 -source 7)

set(SRC
    green/spiky/mc/EntryPoint.java
    green/spiky/mc/launcher/Launcher.java
    green/spiky/mc/launcher/impl/AbstractLauncher.java
    green/spiky/mc/launcher/impl/StandardLauncher.java
    green/spiky/mc/exception/ParameterNotFoundException.java
    green/spiky/mc/exception/ParseException.java
    green/spiky/mc/utils/Parameters.java
    green/spiky/mc/utils/ReflectionUtils.java
    green/spiky/mc/utils/logging/Level.java
    green/spiky/mc/utils/logging/Log.java
    green/spiky/mc/legacy/LegacyProxy.java
)

set(LEGACY_SRC
    legacy/green/spiky/mc/legacy/LegacyFrame.java
    legacy/green/spiky/mc/legacy/LegacyLauncher.java
    legacy/green/spiky/mc/legacy/fix/online/Handler.java
    legacy/green/spiky/mc/legacy/fix/online/OnlineFixes.java
    legacy/green/spiky/mc/legacy/fix/online/OnlineModeFix.java
    legacy/green/spiky/mc/legacy/fix/online/SkinFix.java
    legacy/green/spiky/mc/legacy/utils/Base64.java
    legacy/green/spiky/mc/legacy/utils/api/MojangApi.java
    legacy/green/spiky/mc/legacy/utils/api/Texture.java
    legacy/green/spiky/mc/legacy/utils/json/JsonParseException.java
    legacy/green/spiky/mc/legacy/utils/json/JsonParser.java
    legacy/green/spiky/mc/legacy/utils/url/ByteArrayUrlConnection.java
    legacy/green/spiky/mc/legacy/utils/url/UrlUtils.java
    legacy/net/minecraft/Launcher.java
    legacy/green/spiky/mc/legacy/LegacyProxy.java
)

add_jar(NewLaunch ${SRC})
add_jar(NewLaunchLegacy ${LEGACY_SRC} INCLUDE_JARS NewLaunch)
install_jar(NewLaunch "${JARS_DEST_DIR}")
install_jar(NewLaunchLegacy "${JARS_DEST_DIR}")
