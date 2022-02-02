export function isJsonString(str: string): boolean {
    try {
       const o = JSON.parse(str);
        if (o && typeof o === "object") {
            return o;
        }
    } catch (e) {
        return false;
    }
    return true;
}