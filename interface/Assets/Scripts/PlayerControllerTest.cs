using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerControllerTest : MonoBehaviour
{
    public float speed = 3;

    public GameObject jamSignal;
    public GameObject RecoverAfterBattle;

    private float _x;
    private float _y;

    private Vector2 _currentInput;

    private bool _isMoving;

    private Animator _animator;

    private GameObject _player;
    private GameObject _EnergyBall;

    private SpriteRenderer _spriteRenderer1;
    private SpriteRenderer _spriteRenderer2;
    private SpriteRenderer _spriteRenderer3;

    // Start is called before the first frame update
    void Start()
    {
        _player = transform.GetChild(0).gameObject;
        _EnergyBall = transform.GetChild(1).gameObject;

        _animator = _player.GetComponent<Animator>();

        _spriteRenderer1 = GetComponent<SpriteRenderer>();
        _spriteRenderer2 = _player.GetComponent<SpriteRenderer>();
        _spriteRenderer3 = _EnergyBall.GetComponent<SpriteRenderer>();
    }

    // Update is called once per frame
    void Update()
    {
        _x = Input.GetAxis("Horizontal");
        _y = Input.GetAxis("Vertical");

        _currentInput.Set(_x, _y);

        if (Mathf.Approximately(_currentInput.x * speed, 0) && Mathf.Approximately(_currentInput.y * speed, 0))
        {
            _isMoving = false;
        }
        else
        {
            _isMoving = true;
        }

        _animator.SetBool("isMoving", _isMoving);

        if (Input.GetMouseButtonDown(0))
        {
            //GameObject _jamSignal = Instantiate(jamSignal, transform);
            //GameObject _recoverAfterBattle = Instantiate(RecoverAfterBattle, transform);
            _spriteRenderer1.color = new Color(1, 1, 1, 0);
            _spriteRenderer2.color = new Color(1, 1, 1, 0.5f);
            _spriteRenderer3.color = new Color(1, 1, 1, 0.5f);
        }
    }

    public void FixedUpdate()
    {
        Vector2 position = transform.position;
        position += speed * _currentInput * Time.deltaTime;
        transform.position = position;

        _EnergyBall.transform.RotateAround(transform.position, Vector3.up, 100 * Time.deltaTime);
        _EnergyBall.transform.Rotate(0, -100 * Time.deltaTime, 0);
    }
}
