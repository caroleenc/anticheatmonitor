import csv
import re

def parse_log_file(file_path):
    detail_array = []
    with open(file_path, 'r') as csvfile:
        reader = csv.DictReader(csvfile)

        for row in reader:
            # Use the correct key names from the CSV (including spaces)
            time_of_day = row.get('Time of Day')
            process_name = row.get('Process Name')
            pid = row.get('PID')
            operation = row.get('Operation')
            path = row.get('Path')
            result = row.get('Result')
            detail = row.get('Detail')

            pattern = r"([\w\s]+):\s*\"?([^\",]+)\"?"
            matches = re.findall(pattern, detail)
            
            # print(matches)

            # Convert matches to a dictionary
            for key, value in matches:
                # if key.strip() not in detail_array:
                #     detail_array.append(key.strip())
                if ":" in value:
                  if key.strip() not in detail_array:
                        detail_array.append(key.strip())
    # for val in detail_array:
        # print('{"mutate": \{"rename": {"', val, '": "[Detail][', val, ']"}}},"')
    print(detail_array)
    
# details = ['Parent PID', 'Command line', 'Current directory', 'Environment', 'C', 'Thread ID', 'Image Base', 'Image Size', 'Desired Access', 'Disposition', 'Options', 'Attributes', 'ShareMode', 'AllocationSize', 'OpenResult', 'EndOfFile', 'NumberOfLinks', 'DeletePending', 'Directory', 'Offset', 'Length', 'Priority', 'Type', 'Data', 'CreationTime', 'LastAccessTime', 'LastWriteTime', 'ChangeTime', 'FileAttributes', 'SyncType', 'PageProtection', 'Granted Access', 'Query', 'HandleTags', 'Name', 'ReparseTag', 'Flags', 'Index', 'SubKeys', 'Values', 'KeySetInformationClass', 'seqnum', 'connid', 'Information', 'FileInformationClass', 'Filter', '2', '1', '3', '4', 'VolumeCreationTime', 'VolumeSerialNumber', 'SupportsObjects', 'VolumeLabel', 'User Time', 'Kernel Time', 'Control', 'O Flags', 'Exit Status', 'Private Bytes', 'Peak Private Bytes', 'Working Set', 'Peak Working Set']

# for val in details:
#     # print('{"mutate": {"rename": {"', val, '": "[Detail][', val, ']"}}},')
#     print('if [Detail]['+val.strip()+'] { mutate { rename => {"Detail.' +val.strip()+'" => "'+val.strip()+'" }}}')
# Example log file path
input_csv_file = './be_unturned_1.CSV'

# Call the function with the log file
parse_log_file(input_csv_file)