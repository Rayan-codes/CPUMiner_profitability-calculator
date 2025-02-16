mport requests
import re

# Minerstat API URL
MINERSTAT_API_URL = "https://api.minerstat.com/v2/coins"

# Function to fetch mining profitability data from Minerstat
def fetch_profitability_data():
    try:
        response = requests.get(MINERSTAT_API_URL)
        response.raise_for_status()
        data = response.json()

        if not data:
            print("❌ No coin data found in Minerstat API response!")
        else:
            print(f"✅ Retrieved mining profitability for {len(data)} coins.")

        return data
    except requests.exceptions.RequestException as e:
        print(f"❌ API Fetch Error: {e}")
        return []

# Function to extract hashrates from hashrates.txt
def load_hashrates():
    hashrates = {}
    try:
        with open("hashrates.txt", "r", encoding="utf-8") as file:
            for line in file:
                match = re.search(r"Miner: (.+?) -> Average Hashrate: ([\d.]+) MH/s", line)
                if match:
                    miner_file = match.group(1)
                    hashrate = float(match.group(2))
                    coin_name = miner_file.split("-")[-1].replace(".bat", "").strip()
                    hashrates[coin_name.lower()] = hashrate
        return hashrates
    except FileNotFoundError:
        print("❌ hashrates.txt not found!")
        return {}

# Function to calculate profits
def calculate_profits(hashrates, profitability_data):
    results = []
    for coin, hashrate in hashrates.items():
        matched = False
        for coin_data in profitability_data:
            if "coin" in coin_data and "tag" in coin_data and "reward" in coin_data and "price" in coin_data:
                if coin.lower() in [coin_data["coin"].lower(), coin_data["tag"].lower()]:
                    daily_profit = hashrate * float(coin_data["reward"]) * float(coin_data["price"])  # Profit formula
                    monthly_profit = daily_profit * 30

                    result = f"Coin: {coin_data['coin']} ({coin_data['tag']})\n"
                    result += f"  Hashrate: {hashrate} MH/s\n"
                    result += f"  Daily Profit: ${daily_profit:.2f}\n"
                    result += f"  Monthly Profit: ${monthly_profit:.2f}\n"
                    results.append(result)
                    matched = True
                    break
            else:
                print(f"⚠️ Skipping {coin_data}: Missing required fields.")

        if not matched:
            results.append(f"⚠️ Skipping {coin}: No data found in Minerstat.")

    return results

# Main function
def main():
    print("🔄 Fetching latest mining profitability data from Minerstat...")
    profitability_data = fetch_profitability_data()
    
    print("🔄 Loading hashrates from file...")
    hashrates = load_hashrates()

    if not hashrates:
        print("⚠️ No hashrates found. Exiting.")
        return

    print("🔄 Calculating profits...")
    results = calculate_profits(hashrates, profitability_data)

    # Save to profit.txt
    with open("profit.txt", "w", encoding="utf-8") as profit_file:
        profit_file.write("\n".join(results))

    print("✅ Done! Results saved to profit.txt.")

# Run the script
if __name__ == "__main__":
    main()
