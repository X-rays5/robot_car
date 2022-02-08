export function isJsonString(str: string): boolean {
    if (typeof str !== 'string') return false;

    try {
        const res = JSON.parse(str);
        return (typeof res === "object" && res !== null);
    } catch (e) {
        return false;
    }
}