import os
import os.path
import paramiko
import sys
import getopt
import json

config = {
    'ip' : '',
    'username' : 'ubuntu',
    'password' : '',
    'port' : 22,
    'upload_path' : '/home/ubuntu/THUAI5_for_Windows'
}

config['cwd'] = os.getcwd()

dirs = []
file_list = []
folder_list = []
remote_file_list =[]
file_list_json =[]

def upload_file(local_path, remote_path, ssh, sftp):
    """上传文件"""
    stdin, stdout, stderr = ssh.exec_command('find ' + remote_path)
    res = stdout.read().decode('utf-8')
    if len(res) == 0:
        sftp.put(local_path, remote_path)
    else :
        local_file_size = os.path.getsize(local_path)
        stdin, stdout, stderr = ssh.exec_command('du -b ' + remote_path)
        result = stdout.read().decode('utf-8')
        remote_file_size = int(result.split('\t')[0])
        if local_file_size != remote_file_size:
            sftp.put(local_path, remote_path)


def mkdir(dir, ssh):
    """创建文件夹"""
    stdin, stdout, stderr = ssh.exec_command('find ' + dir)
    res = stdout.read().decode('utf-8')
    if len(res) == 0:
        ssh.exec_command('mkdir ' + dir)

if __name__ == '__main__':
    """读取命令行参数"""
    try:
        print(sys.argv[1:])
        opts, dirs = getopt.getopt(sys.argv[1:], "i:k:", ["id=", "key="])
    except getopt.GetoptError:
        print('Error: --id <secret_id> --key <secret_key>')
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-i", "--id"):
            config['ip'] = arg
        elif opt in ("-k", "--key"):
            config['password'] = arg

    """创建ssh控制台"""
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname=config['ip'], port=config['port'], username=config['username'], password=config['password'])
    t = paramiko.Transport(sock=(config['ip'], config['port']))
    t.connect(username=config['username'], password=config['password'])
    sftp = paramiko.SFTPClient.from_transport(t)

    """检查根目录是否存在"""
    root_path = config['upload_path']
    mkdir(root_path, ssh)

    """初始化文件列表"""
    for dir in dirs:
        mkdir(config['upload_path'] + '/' + dir, ssh)
        for root, folders, files in os.walk(dir):
            for folder in folders:
                folder_list.append(os.path.join(root, folder))
            for file in files:
                file_list.append(os.path.join(root, file))
    
    """创建ssh控制台"""
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname=config['ip'], port=config['port'], username=config['username'], password=config['password'])
    t = paramiko.Transport(sock=(config['ip'], config['port']))
    t.connect(username=config['username'], password=config['password'])
    sftp = paramiko.SFTPClient.from_transport(t)
    # sftp.put("D:\\fuyh\\THUAI5\\README.md", '/home/ubuntu/README.md')
    
    for folder in folder_list:
        dir = config['upload_path'] + '/' + folder.replace('\\', '/')
        mkdir(dir, ssh)
    
    """上传文件"""
    print("uploading...")
    for file in file_list:
        local_file_path = os.path.join(config['cwd'], file)
        remote_file_path = os.path.join(config['upload_path'], file).replace('\\', '/')
        file_list_json.append(remote_file_path)
        upload_file(local_file_path, remote_file_path, ssh, sftp)
    
    with open('./file.json', 'w') as f:
        json.dump(file_list_json, f)
    upload_file('./file.json', config['upload_path'] + '/file.json', ssh, sftp)

    os.remove('./file.json')
    print("Finished")