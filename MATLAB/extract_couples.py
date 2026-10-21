import csv

def csv_to_spice_table(file_path, col_x_name, col_y_name):
    pairs = []
    try:
        # utf-8-sig handles special characters exported by Excel
        with open(file_path, mode='r', encoding='utf-8-sig') as file:
            reader = csv.DictReader(file)
            for row in reader:
                try:
                    # Convert strings to numbers and round them
                    x_val = float(row[col_x_name])
                    y_val = float(row[col_y_name])
                    pairs.append(f"({round(x_val, 4)}, {round(y_val, 4)})")
                except (ValueError, KeyError):
                    continue # Skip empty rows or text
                    
        table_values = " ".join(pairs)
        return f"B1 n1 n2 V=TABLE {{V(ctrl)}} = {table_values}"
        
    except FileNotFoundError:
        return "Error: CSV file not found. Make sure the name matches exactly."

# Update these three variables to match your file exactly
CSV_FILE = "h100_data.csv"   
X_COLUMN = "Voltage"               
Y_COLUMN = "Current"               

# Generate and print the output
spice_command = csv_to_spice_table(CSV_FILE, X_COLUMN, Y_COLUMN)
print(spice_command)
